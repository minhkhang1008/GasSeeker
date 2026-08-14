#include "bench.h"

#include "../core/config.h"
#include "../core/gas.h"
#include "../lora/lora_link.h"
#include "hw_imu.h"
#include "hw_io.h"
#include "hw_motors.h"
#include "motion.h"
#include "odometry.h"
#include "robot_io.h"

namespace bench {

enum class Mode : uint8_t { NONE, RAW_MOTOR, DRIVE, TURN, ENC, BUMP, GAS, SNIFF, PING, SELFTEST };

static RobotIO* io_ = nullptr;
static Mode mode_ = Mode::NONE;
static uint32_t deadline_ = 0;
static uint32_t next_print_ = 0;
static float want_ = 0.0f;          // cm hoac do da yeu cau
static long tick0_l_ = 0, tick0_r_ = 0;
static gs::Sniffer sniffer_;
static uint8_t ping_left_ = 0;

// --- trang thai cua selftest ---
static uint8_t st_step_ = 0;
static uint32_t st_t0_ = 0;
static int st_fail_ = 0, st_warn_ = 0;
static long st_tick0_l_ = 0, st_tick0_r_ = 0;
static double st_gyro_acc_ = 0.0;
static uint32_t st_gyro_n_ = 0;

void begin(RobotIO* io) { io_ = io; }
bool active() { return mode_ != Mode::NONE; }

void printHelp() {
  Serial.println("--- che do kiem tra phan cung ---");
  Serial.println("  mot <L> <R>   cap PWM tho 1,5 s (-255..255)");
  Serial.println("  drive <cm>    di thang roi bao odometry");
  Serial.println("  turn <deg>    quay mot goc roi bao ket qua");
  Serial.println("  enc           in xung encoder 20 s");
  Serial.println("  bump          in cong tac va cham 20 s");
  Serial.println("  gas           in bang ADC/Rs/ppm 60 s (de ve dac tuyen)");
  Serial.println("  sniff         mot phep dung-ngui, in mot dong");
  Serial.println("  selftest      kiem tra lan luot moi khoi, in bang PASS/FAIL");
  Serial.println("  ping          gui 5 goi LoRa thu");
  Serial.println("  bench off     dung");
}

void abort() {
  if (mode_ == Mode::NONE) return;
  hw::motionStop();
  hw::motorsSet(0, 0);
  hw::motorsEnable(false);
  mode_ = Mode::NONE;
  Serial.println("[bench] da dung.");
}

// --------------------------------------------------------------------------
static void startMotion(Mode m, float value) {
  hw::motorsEnable(true);
  hw::odomSegmentReset();
  tick0_l_ = hw::odomTicksL();
  tick0_r_ = hw::odomTicksR();
  want_ = value;
  mode_ = m;
  if (m == Mode::DRIVE) {
    Serial.printf("[bench] di thang %.1f cm...\n", value);
    hw::motionForward(value);
  } else {
    Serial.printf("[bench] quay %.1f do...\n", value);
    hw::motionTurn(value);
  }
}

static void reportMotion() {
  const long dl = hw::odomTicksL() - tick0_l_;
  const long dr = hw::odomTicksR() - tick0_r_;
  const gs::Pose p = hw::odomPose();

  if (mode_ == Mode::DRIVE) {
    const float got = hw::odomSegmentCm();
    Serial.printf("[bench] xong. Odometry bao %.1f cm (yeu cau %.1f).\n", got, want_);
    Serial.printf("        xung L=%ld R=%ld\n", dl, dr);
    Serial.println("        DO BANG THUOC quang duong that, roi sua config.h:");
    Serial.printf("        WHEEL_DIAMETER_MM_moi = %.1f * (do_duoc_cm / %.1f)\n",
                  cfg::WHEEL_DIAMETER_MM, got > 0.1f ? got : 1.0f);
    if (!hw::odomHasRightEncoder()) {
      if (dl < 3)
        Serial.println("        !! encoder khong ra xung nao - kiem tra HC-020K tren GPIO1");
      else
        Serial.println("        (cau hinh mot encoder: quang duong lay tu banh sau TRAI)");
    } else if (dl > 0 && dr > 0) {
      const float ratio = (float)dl / (float)dr;
      if (ratio < 0.9f || ratio > 1.1f)
        Serial.printf("        !! hai banh lech %.0f%% - kiem tra encoder hoac ma sat\n",
                      fabsf(ratio - 1.0f) * 100.0f);
    } else {
      Serial.println("        !! mot ben KHONG co xung nao - kiem tra encoder ben do");
    }
  } else {
    Serial.printf("[bench] xong. Gyro bao da quay %.1f do (yeu cau %.1f), "
                  "huong hien tai %.1f do.\n",
                  hw::odomSegmentTurnDeg(), want_, p.heading_deg);
    Serial.printf("        xung L=%ld R=%ld\n", dl, dr);
    Serial.println("        So sanh voi goc THAT quay duoc. Lech nhieu -> xem lai");
    Serial.println("        bias gyro (khoi dong lai va giu yen xe) hoac TURN_KP.");
  }
  hw::motorsEnable(false);
  mode_ = Mode::NONE;
}

// --------------------------------------------------------------------------
bool handleCommand(const char* cmd) {
  if (strncmp(cmd, "bench", 5) == 0) {
    if (strstr(cmd, "off")) abort();
    else printHelp();
    return true;
  }

  if (strncmp(cmd, "mot", 3) == 0) {
    int l = 0, r = 0;
    if (sscanf(cmd + 3, "%d %d", &l, &r) != 2) {
      Serial.println("[bench] dung: mot <L> <R>   vd: mot 120 120");
      return true;
    }
    l = gs::clampv(l, -cfg::PWM_MAX, cfg::PWM_MAX);
    r = gs::clampv(r, -cfg::PWM_MAX, cfg::PWM_MAX);
    Serial.printf("[bench] PWM tho L=%d R=%d trong 1,5 s. Xem xe co quay dung chieu.\n", l, r);
    hw::motorsEnable(true);
    hw::odomSetWheelDir(l > 0 ? 1 : (l < 0 ? -1 : 0), r > 0 ? 1 : (r < 0 ? -1 : 0));
    hw::motorsSet(l, r);
    mode_ = Mode::RAW_MOTOR;
    deadline_ = millis() + 1500;
    return true;
  }

  if (strncmp(cmd, "drive", 5) == 0) {
    startMotion(Mode::DRIVE, atof(cmd + 5));
    return true;
  }
  if (strncmp(cmd, "turn", 4) == 0) {
    startMotion(Mode::TURN, atof(cmd + 4));
    return true;
  }

  if (strncmp(cmd, "enc", 3) == 0) {
    Serial.println("[bench] day banh bang tay, xem xung co tang deu khong (20 s).");
    mode_ = Mode::ENC;
    deadline_ = millis() + 20000;
    next_print_ = 0;
    return true;
  }
  if (strncmp(cmd, "bump", 4) == 0) {
    Serial.println("[bench] bam thu hai cong tac va cham (20 s).");
    mode_ = Mode::BUMP;
    deadline_ = millis() + 20000;
    next_print_ = 0;
    return true;
  }

  if (strncmp(cmd, "gas", 3) == 0) {
    Serial.println("[bench] bang do khi trong 60 s. Cot cach nhau bang TAB,");
    Serial.println("        copy thang vao bang tinh de ve duong dac tuyen.");
    Serial.println("t_s\tadc\tmv\tnorm\tRs_ohm\tratio\tppm\tmuc");
    mode_ = Mode::GAS;
    deadline_ = millis() + 60000;
    next_print_ = 0;
    return true;
  }

  if (strncmp(cmd, "sniff", 5) == 0) {
    if (!io_) return true;
    Serial.println("[bench] dung ngui... giu xe yen.");
    hw::motionStop();
    sniffer_.start(millis());
    mode_ = Mode::SNIFF;
    return true;
  }

  if (strncmp(cmd, "selftest", 8) == 0) {
    Serial.println();
    Serial.println("=== TU KIEM TRA PHAN CUNG ===");
    Serial.println("!! KE XE LEN CAO, banh khong cham dat. Giu xe YEN.");
    Serial.println("   Bat dau sau 3 giay...");
    st_step_ = 0;
    st_fail_ = st_warn_ = 0;
    st_t0_ = millis();
    mode_ = Mode::SELFTEST;
    return true;
  }

  if (strncmp(cmd, "ping", 4) == 0) {
    if (!radiolink::ok()) {
      Serial.println("[bench] LoRa khong dung duoc, xem lai day SPI va tu nguon.");
      return true;
    }
    Serial.println("[bench] gui 5 goi thu, xem RSSI ben tram thu.");
    ping_left_ = 5;
    mode_ = Mode::PING;
    next_print_ = 0;
    return true;
  }

  return false;
}

// --------------------------------------------------------------------------
static void stResult(const char* name, int level, const char* detail) {
  // level: 0 = dat, 1 = canh bao, 2 = hong
  const char* tag = (level == 0) ? "  DAT " : (level == 1 ? " CANH " : " HONG ");
  if (level == 1) ++st_warn_;
  if (level == 2) ++st_fail_;
  Serial.printf("[%s] %-22s %s\n", tag, name, detail);
}

// Moi buoc tra ve true khi da xong de chuyen sang buoc ke tiep.
static bool selftestStep(uint32_t elapsed) {
  char msg[96];
  switch (st_step_) {
    case 0:  // cho nguoi dung ke xe len
      return elapsed >= 3000;

    case 1:  // I2C / MPU6050
      if (hw::imuOk())
        stResult("MPU6050 (I2C)", 0, "tra loi tren bus I2C");
      else
        stResult("MPU6050 (I2C)", 2, "KHONG thay - kiem tra SDA=GPIO8, SCL=GPIO9, nguon 3V3");
      return true;

    case 2:  // gyro dung yen phai gan 0
      if (!hw::imuOk()) return true;
      st_gyro_acc_ += hw::imuGyroZ();
      ++st_gyro_n_;
      if (elapsed < 1200) return false;
      {
        const float avg = (float)(st_gyro_acc_ / (double)st_gyro_n_);
        snprintf(msg, sizeof(msg), "xe dung yen doc %.2f do/s (bias %.2f)", avg, hw::imuBias());
        stResult("Gyro luc dung yen", fabsf(avg) < 1.5f ? 0 : 1, msg);
      }
      return true;

    case 3:  // ADC cua MQ-3
      {
        const uint16_t adc = io_ ? io_->lastAdc() : 0;
        snprintf(msg, sizeof(msg), "ADC = %u", adc);
        if (adc < 30)
          stResult("MQ-3 (ADC)", 2, "gia tri ~0 - kiem tra day AO va mach chia ap");
        else if (adc > 4000)
          stResult("MQ-3 (ADC)", 2, "bao hoa - mach chia ap sai ti so hoac AO cham 5V");
        else
          stResult("MQ-3 (ADC)", 0, msg);
      }
      return true;

    case 4:  // cong tac va cham
      if (hw::bumperAny())
        stResult("Cong tac va cham", 1, "DANG bi cham - dang bam hay dau nguoc?");
      else
        stResult("Cong tac va cham", 0, "ca hai deu ho (dung)");
      return true;

    case 5:  // LoRa
      if (radiolink::ok())
        stResult("LoRa SX1262", 0, "khoi tao thanh cong");
      else
        stResult("LoRa SX1262", 2, radiolink::lastError());
      return true;

    case 6:  // banh trai
      if (elapsed == 0) {}
      if (elapsed < 50) {
        st_tick0_l_ = hw::odomTicksL();
        st_tick0_r_ = hw::odomTicksR();
        hw::motorsEnable(true);
        hw::motorsSet(150, 0);
        return false;
      }
      if (elapsed < 800) return false;
      hw::motorsSet(0, 0);
      {
        const long dl = hw::odomTicksL() - st_tick0_l_;
        const long dr = hw::odomTicksR() - st_tick0_r_;
        snprintf(msg, sizeof(msg), "banh trai quay, %ld xung", dl);
        if (dl < 3)
          stResult("Motor + encoder TRAI", 2, "khong co xung - motor khong quay hoac encoder hong");
        else if (hw::odomHasRightEncoder() && dr > 2)
          stResult("Motor + encoder TRAI", 1, "banh PHAI cung co xung - hai encoder dau lan nhau?");
        else
          stResult("Motor + encoder TRAI", 0, msg);
      }
      return true;

    case 7:  // banh phai
      if (elapsed < 50) {
        st_tick0_l_ = hw::odomTicksL();
        st_tick0_r_ = hw::odomTicksR();
        hw::motorsSet(0, 150);
        return false;
      }
      if (elapsed < 800) return false;
      hw::motorsSet(0, 0);
      hw::motorsEnable(false);
      {
        const long dl = hw::odomTicksL() - st_tick0_l_;
        const long dr = hw::odomTicksR() - st_tick0_r_;
        if (!hw::odomHasRightEncoder()) {
          // Cau hinh mot encoder: khong do duoc ben phai, phai xac nhan bang mat.
          snprintf(msg, sizeof(msg), "da cap dien 0,8 s - HAY NHIN xem 2 banh phai co quay khong");
          stResult("Motor PHAI (khong encoder)", 0, msg);
          if (dl > 2)
            stResult("Canh bao day", 1, "chay motor PHAI ma encoder TRAI co xung - dau nham ben?");
        } else if (dr < 3) {
          stResult("Motor + encoder PHAI", 2, "khong co xung - motor khong quay hoac encoder hong");
        } else if (dl > 2) {
          stResult("Motor + encoder PHAI", 1, "banh TRAI cung co xung - hai encoder dau lan nhau?");
        } else {
          snprintf(msg, sizeof(msg), "banh phai quay, %ld xung", dr);
          stResult("Motor + encoder PHAI", 0, msg);
        }
      }
      return true;

    default:
      return true;
  }
}

void update() {
  if (mode_ == Mode::NONE) return;
  const uint32_t now = millis();

  switch (mode_) {
    case Mode::RAW_MOTOR:
      if (now >= deadline_) {
        hw::motorsSet(0, 0);
        hw::motorsEnable(false);
        hw::odomSetWheelDir(0, 0);
        Serial.printf("[bench] xong. xung L=%ld R=%ld\n", hw::odomTicksL(), hw::odomTicksR());
        mode_ = Mode::NONE;
      }
      break;

    case Mode::DRIVE:
    case Mode::TURN:
      if (!hw::motionBusy()) reportMotion();
      break;

    case Mode::ENC:
      if (now >= next_print_) {
        next_print_ = now + 300;
        Serial.printf("  xung L=%-6ld R=%-6ld  quang duong=%.1f cm  huong=%.1f do\n",
                      hw::odomTicksL(), hw::odomTicksR(), hw::odomTravelledCm(),
                      hw::odomPose().heading_deg);
      }
      if (now >= deadline_) mode_ = Mode::NONE;
      break;

    case Mode::BUMP:
      if (now >= next_print_) {
        next_print_ = now + 250;
        Serial.printf("  cong tac trai=%s  phai=%s\n",
                      hw::bumperLeft() ? "CHAM" : "----",
                      hw::bumperRight() ? "CHAM" : "----");
      }
      if (now >= deadline_) mode_ = Mode::NONE;
      break;

    case Mode::GAS:
      if (io_ && now >= next_print_) {
        next_print_ = now + 500;
        const gs::GasReading g = io_->gas();
        const float r0 = io_->r0();
        Serial.printf("%.1f\t%u\t%.0f\t%d\t%.0f\t%.3f\t%.0f\t%s\n", now / 1000.0f,
                      io_->lastAdc(), io_->lastMv(), g.normalized, io_->rs(),
                      r0 > 0 ? io_->rs() / r0 : 0.0f, g.ppm,
                      gs::alarmLevelName(g.level));
      }
      if (now >= deadline_) {
        Serial.println("[bench] het bang do.");
        mode_ = Mode::NONE;
      }
      break;

    case Mode::SNIFF:
      if (io_ && sniffer_.update(*io_)) {
        Serial.printf("[bench] SNIFF: norm=%d  adc=%u  ppm~%.0f   "
                      "(ghi lai kem khoang cach tu nguon)\n",
                      sniffer_.value(), sniffer_.rawValue(), sniffer_.ppm());
        mode_ = Mode::NONE;
      }
      break;

    case Mode::PING:
      if (now >= next_print_ && !radiolink::busy()) {
        next_print_ = now + 1000;
        char body[32];
        const int n = snprintf(body, sizeof(body), "PING,%u", (unsigned)(6 - ping_left_));
        uint8_t ck = 0;
        for (int i = 0; i < n; ++i) ck ^= (uint8_t)body[i];
        char msg[48];
        snprintf(msg, sizeof(msg), "$%s*%02X", body, ck);
        radiolink::send(msg);
        Serial.printf("  da gui goi %d/5: %s\n", 6 - ping_left_, msg);
        if (--ping_left_ == 0) {
          Serial.println("[bench] xong. Ben tram thu phai in 5 dong PING kem RSSI.");
          mode_ = Mode::NONE;
        }
      }
      break;

    case Mode::SELFTEST: {
      if (!selftestStep(now - st_t0_)) break;
      ++st_step_;
      st_t0_ = now;
      st_gyro_acc_ = 0.0;
      st_gyro_n_ = 0;
      if (st_step_ > 7) {
        Serial.println("-----------------------------------------------------");
        if (st_fail_ == 0 && st_warn_ == 0) {
          Serial.println("TAT CA DEU DAT. Sang buoc hieu chuan: 'drive 100'.");
        } else {
          Serial.printf("%d loi, %d canh bao. Sua het loi truoc khi chay thuat toan.\n",
                        st_fail_, st_warn_);
          Serial.println("Tra bang su co o cuoi docs/RUNBOOK.md.");
        }
        Serial.println("-----------------------------------------------------");
        hw::motorsEnable(false);
        mode_ = Mode::NONE;
      }
      break;
    }

    default:
      mode_ = Mode::NONE;
      break;
  }
}

}  // namespace bench
