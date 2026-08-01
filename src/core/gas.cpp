#include "gas.h"

#include <cmath>

namespace gs {

const char* alarmLevelName(AlarmLevel lv) {
  switch (lv) {
    case AlarmLevel::Safe: return "SAFE";
    case AlarmLevel::Detected: return "DETECTED";
    case AlarmLevel::High: return "HIGH";
    case AlarmLevel::Critical: return "CRITICAL";
  }
  return "?";
}

// ---------------------------------------------------------------------------
float GasProcessor::adcToMv(uint16_t adc) {
  return (float)adc / cfg::ADC_MAX_COUNT * cfg::ADC_REF_MV;
}

// Buoc 1 cua de cuong muc 11.1b: tinh dien tro cam bien Rs.
//   mv la dien ap SAU mach chia ap -> phai nhan nguoc lai de ra Vout cua MQ-3.
float GasProcessor::mvToRs(float mv) {
  float vout = (mv / 1000.0f) / cfg::DIV_GAIN;  // dien ap thuc tai chan AO
  if (vout < 0.005f) vout = 0.005f;             // chan chia cho 0
  if (vout > cfg::MQ3_VC_V - 0.005f) vout = cfg::MQ3_VC_V - 0.005f;
  return (cfg::MQ3_VC_V - vout) / vout * cfg::MQ3_RL_OHM;
}

// Buoc 3: quy doi theo duong dac tuyen log-log cua datasheet.
float GasProcessor::rsToPpm(float rs, float r0) {
  if (r0 <= 0.0f || rs <= 0.0f) return 0.0f;
  const float ratio = rs / r0;
  const float ppm = cfg::MQ3_CURVE_A * std::pow(ratio, cfg::MQ3_CURVE_B);
  if (!std::isfinite(ppm) || ppm < 0.0f) return 0.0f;
  if (ppm > 20000.0f) return 20000.0f;  // chan gia tri vo ly khi Rs qua nho
  return ppm;
}

AlarmLevel GasProcessor::ppmToLevel(float ppm) {
  if (ppm >= cfg::PPM_T3) return AlarmLevel::Critical;
  if (ppm >= cfg::PPM_T2) return AlarmLevel::High;
  if (ppm >= cfg::PPM_T1) return AlarmLevel::Detected;
  return AlarmLevel::Safe;
}

// ---------------------------------------------------------------------------
void GasProcessor::begin(uint32_t now_ms) {
  idx_ = 0;
  filled_ = 0;
  sum_ = 0;
  for (int i = 0; i < cfg::GAS_MA_WINDOW; ++i) buf_[i] = 0;
  t_start_ = now_ms;
  baseline_ready_ = false;
  baseline_ = 0;
  base_acc_ = 0.0;
  base_rs_acc_ = 0.0;
  base_n_ = 0;
  r0_ = cfg::MQ3_R0_OHM;
  out_ = GasReading{};
}

void GasProcessor::addSample(uint16_t adc, float mv, uint32_t now_ms) {
  // --- trung binh truot N mau ---
  sum_ -= buf_[idx_];
  buf_[idx_] = adc;
  sum_ += adc;
  idx_ = (idx_ + 1) % cfg::GAS_MA_WINDOW;
  if (filled_ < cfg::GAS_MA_WINDOW) ++filled_;

  const uint16_t raw = (uint16_t)(sum_ / (uint32_t)filled_);
  const float mv_used = (mv > 0.0f) ? mv : adcToMv(raw);
  rs_ = mvToRs(mv_used);

  // --- giai doan do baseline / hieu chuan R0 ---
  if (!baseline_ready_) {
    const uint32_t elapsed = now_ms - t_start_;
    // Bo qua 1 giay dau cho cua so trung binh truot day.
    if (elapsed > 1000) {
      base_acc_ += raw;
      base_rs_acc_ += rs_;
      ++base_n_;
    }
    if (elapsed >= cfg::BASELINE_MS && base_n_ > 0) {
      baseline_ = (uint16_t)(base_acc_ / (double)base_n_);
      if (cfg::MQ3_R0_OHM <= 0.0f) {
        // Tu hieu chuan: R0 = Rs_khong_khi_sach / RATIO_CLEAN_AIR
        r0_ = (float)(base_rs_acc_ / (double)base_n_) / cfg::MQ3_RATIO_CLEAN_AIR;
      }
      baseline_ready_ = true;
    }
  }

  // --- ket qua ---
  out_.raw = raw;
  out_.normalized = baseline_ready_ ? (int16_t)((int32_t)raw - (int32_t)baseline_) : 0;
  out_.ppm = rsToPpm(rs_, r0_);
  out_.level = ppmToLevel(out_.ppm);
  out_.valid = baseline_ready_;
}

// ---------------------------------------------------------------------------
void Sniffer::start(uint32_t now_ms) {
  active_ = true;
  t0_ = now_ms;
  acc_ = acc_raw_ = acc_ppm_ = 0.0;
  n_ = 0;
}

bool Sniffer::update(const IRobot& r) {
  if (!active_) return false;
  const uint32_t dt = r.nowMs() - t0_;

  if (dt >= cfg::SNIFF_SETTLE_MS) {
    const GasReading g = r.gas();
    acc_ += g.normalized;
    acc_raw_ += g.raw;
    acc_ppm_ += g.ppm;
    ++n_;
  }

  if (dt >= cfg::SNIFF_TOTAL_MS) {
    if (n_ > 0) {
      value_ = (int16_t)(acc_ / (double)n_);
      raw_ = (uint16_t)(acc_raw_ / (double)n_);
      ppm_ = (float)(acc_ppm_ / (double)n_);
    }
    active_ = false;
    return true;
  }
  return false;
}

}  // namespace gs
