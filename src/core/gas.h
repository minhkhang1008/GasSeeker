#pragma once
#include <cstdint>

#include "config.h"
#include "irobot.h"

namespace gs {

class GasProcessor {
 public:
  void begin(uint32_t now_ms);

  void addSample(uint16_t adc, float mv, uint32_t now_ms);

  GasReading reading() const { return out_; }
  bool baselineReady() const { return baseline_ready_; }
  uint16_t baseline() const { return baseline_; }
  float r0() const { return r0_; }
  float rsNow() const { return rs_; }

  void setBaseline(uint16_t b) { baseline_ = b; baseline_ready_ = true; }
  void setR0(float r0) { r0_ = r0; }

  static float adcToMv(uint16_t adc);
  static float mvToRs(float mv);
  static float rsToPpm(float rs, float r0);
  static AlarmLevel ppmToLevel(float ppm);

 private:
  void recompute(uint32_t now_ms);

  uint16_t buf_[cfg::GAS_MA_WINDOW] = {0};
  int idx_ = 0;
  int filled_ = 0;
  uint32_t sum_ = 0;

  uint32_t t_start_ = 0;
  bool baseline_ready_ = false;
  uint16_t baseline_ = 0;
  double base_acc_ = 0.0;
  double base_rs_acc_ = 0.0;
  uint32_t base_n_ = 0;

  float r0_ = cfg::MQ3_R0_OHM;
  float rs_ = 0.0f;
  GasReading out_;
};

class Sniffer {
 public:
  void start(uint32_t now_ms);
  bool update(const IRobot& r);
  bool active() const { return active_; }
  int16_t value() const { return value_; }
  uint16_t rawValue() const { return raw_; }
  float ppm() const { return ppm_; }

 private:
  bool active_ = false;
  uint32_t t0_ = 0;
  double acc_ = 0.0;
  double acc_raw_ = 0.0;
  double acc_ppm_ = 0.0;
  uint32_t n_ = 0;
  int16_t value_ = 0;
  uint16_t raw_ = 0;
  float ppm_ = 0.0f;
};

}
