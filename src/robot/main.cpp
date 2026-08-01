// ============================================================================
//  GasSeeker - firmware tren xe (ESP32-S3)
//
//  Vong doi mot buoi do:
//      CALIB  : say cam bien, do bias gyro, do baseline khong khi sach
//      READY  : cho lenh chay. Nhan giu nut BOOT de doi thuat toan.
//      RUN    : mot trong ba thuat toan dieu khien xe
//      DONE   : da ket luan -> dung, keu coi, van tiep tuc gui telemetry
//
//  Dieu khien:
//      Nut BOOT nhan ngan : READY -> RUN, hoac RUN -> dung khan
//      Nut BOOT nhan giu  : doi thuat toan (chi khi dang o READY/DONE)
//      Serial / LoRa      : start | stop | algo 0|1|2 | cal | info
//
//  Robot chay hoan toan doc lap. Mat LoRa hay rut USB deu khong anh huong
//  toi logic tim nguon (de cuong muc 11.4).
// ============================================================================
#include <Arduino.h>

#include "../core/config.h"
#include "../core/mission.h"
#include "../lora/lora_link.h"
#include "bench.h"
#include "hw_gas.h"
#include "hw_imu.h"
#include "hw_io.h"
#include "hw_motors.h"
#include "motion.h"
#include "odometry.h"
#include "robot_io.h"

enum class Sys : uint8_t { CALIB, READY, RUN, DONE };

static RobotIO io;
static gs::Mission mission;
static Sys sys_ = Sys::CALIB;
static gs::Algo algo_ = gs::Algo::GRADIENT;  // [CHON] mac dinh khi bat may

static uint32_t next_control_ms_ = 0;
static uint32_t next_tele_ms_ = 0;
static uint32_t blink_ms_ = 0;
static bool blink_on_ = false;

// ---------------------------------------------------------------------------
static void banner() {
  Serial.println();
  Serial.println("=====================================================");
  Serial.print("  GasSeeker  ");
  Serial.println(cfg::FW_VERSION);
  Serial.println("  Robot do tim nguon ro ri khi bang cam bien don");
  Serial.println("=====================================================");
  Serial.printf("  San      : %.0f x %.0f cm, o luoi %.0f cm (%d x %d)\n",
                cfg::ARENA_W_CM, cfg::ARENA_H_CM, cfg::CELL_CM, cfg::GRID_NX, cfg::GRID_NY);
  Serial.printf("  MPU6050  : %s\n", hw::imuOk() ? "OK (dung gyro cho huong)"
                                                 : "KHONG THAY -> dung hieu so encoder");
  Serial.printf("  LoRa     : %s (%s)\n", radiolink::ok() ? "OK" : "KHONG DUNG DUOC",
                radiolink::lastError());
  Serial.printf("  Thuat toan: %s\n", gs::algoName(algo_));
  Serial.println("  Lenh: start | stop | algo 0|1|2 | cal | info | bench");
  Serial.println("-----------------------------------------------------");
}

static void printInfo() {
  const gs::GasReading g = io.gas();
  const gs::Pose p = io.pose();
  Serial.printf("ADC=%u mV=%.0f base=%u norm=%d Rs=%.0f R0=%.0f ppm=%.0f muc=%s\n",
                io.lastAdc(), io.lastMv(), io.baseline(), g.normalized, io.rs(), io.r0(),
                g.ppm, gs::alarmLevelName(g.level));
  Serial.printf("pose=(%.1f, %.1f) huong=%.1f do  quang duong=%.0f cm  tick L/R=%ld/%ld\n",
                p.x_cm, p.y_cm, p.heading_deg, io.travelledCm(), hw::odomTicksL(),
                hw::odomTicksR());
  Serial.printf("thuat toan=%s  trang thai=%s  motion=%s\n", gs::algoName(algo_),
                mission.stateName(), hw::motionStateName());
}

// ---------------------------------------------------------------------------
static void startRun() {
  if (bench::active()) {
    Serial.println("!! Dang o che do kiem tra. Go 'bench off' truoc.");
    return;
  }
  if (!io.baselineReady()) {
    Serial.println("!! Chua do xong baseline - go 'cal' roi doi.");
    return;
  }
  hw::odomReset(cfg::START_X_CM, cfg::START_Y_CM, cfg::START_HEADING_DEG);
  io.clearBump();
  io.setMotorsEnabled(true);
  mission.begin(io, algo_);
  sys_ = Sys::RUN;
  hw::beepPattern(1);
  Serial.printf(">>> BAT DAU: %s\n", gs::algoName(algo_));
}

static void stopRun(const char* why) {
  mission.abort(io);
  hw::motionStop();
  io.setMotorsEnabled(false);
  sys_ = Sys::READY;
  Serial.printf("<<< DUNG (%s)\n", why);
}

static void cycleAlgo() {
  algo_ = (gs::Algo)(((int)algo_ + 1) % (int)gs::Algo::COUNT);
  Serial.printf("Thuat toan -> %s\n", gs::algoName(algo_));
  hw::beepPattern((uint8_t)algo_ + 1);
}

// ---------------------------------------------------------------------------
static void handleCommand(const char* cmd) {
  // Che do kiem tra phan cung: khong duoc dung khi thuat toan dang chay.
  if (sys_ != Sys::RUN && bench::handleCommand(cmd)) return;

  if (strncmp(cmd, "start", 5) == 0) {
    if (sys_ == Sys::READY || sys_ == Sys::DONE) startRun();
  } else if (strncmp(cmd, "stop", 4) == 0) {
    stopRun("lenh");
  } else if (strncmp(cmd, "algo", 4) == 0) {
    const int n = atoi(cmd + 4);
    if (n >= 0 && n < (int)gs::Algo::COUNT && sys_ != Sys::RUN) {
      algo_ = (gs::Algo)n;
      Serial.printf("Thuat toan -> %s\n", gs::algoName(algo_));
    }
  } else if (strncmp(cmd, "cal", 3) == 0) {
    if (sys_ != Sys::RUN) {
      Serial.println("Do lai baseline... giu robot trong khong khi sach.");
      io.restartBaseline();
      sys_ = Sys::CALIB;
    }
  } else if (strncmp(cmd, "info", 4) == 0) {
    printInfo();
  }
}

static void pollSerial() {
  static char buf[48];
  static uint8_t n = 0;
  while (Serial.available()) {
    const char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (n > 0) {
        buf[n] = '\0';
        handleCommand(buf);
        n = 0;
      }
    } else if (n < sizeof(buf) - 1) {
      buf[n++] = c;
    }
  }
}

static void pollLoRa() {
  if (!cfg::ENABLE_UPLINK || !radiolink::ok()) return;
  char buf[64];
  if (!radiolink::receive(buf, sizeof(buf))) return;
  // Chi nhan lenh co tien to CMD, de khong an nham goi telemetry cua chinh minh.
  if (strncmp(buf, "CMD,", 4) == 0) {
    Serial.printf("[LoRa] lenh tu tram: %s\n", buf + 4);
    handleCommand(buf + 4);
  }
}

// ---------------------------------------------------------------------------
static void sendTelemetry() {
  gs::TelemetrySample s = mission.sample(io);
  if (sys_ != Sys::RUN && sys_ != Sys::DONE) {
    // Truoc khi chay, van bao ve nong do tai cho: robot la mot tram do di dong.
    const gs::GasReading g = io.gas();
    s.t_ms = millis();
    s.algo = gs::algoShortName(algo_);
    s.state = (sys_ == Sys::CALIB) ? "CALIB" : "READY";
    s.adc = g.raw;
    s.norm = g.normalized;
    s.ppm = g.ppm;
    s.level = g.level;
    s.pose = io.pose();
    s.dist_cm = io.travelledCm();
    s.finished = false;
  }

  char line[160];
  if (gs::buildCsv(line, sizeof(line), s) == 0) return;
  Serial.println(line);
  radiolink::send(line);
}

// ---------------------------------------------------------------------------
static void updateLed() {
  const uint32_t now = millis();
  switch (sys_) {
    case Sys::CALIB:
      if (now - blink_ms_ > 400) {
        blink_ms_ = now;
        blink_on_ = !blink_on_;
        hw::statusColor(0, 0, blink_on_ ? 60 : 0);  // xanh duong nhap nhay
      }
      break;
    case Sys::READY:
      // Mau bao thuat toan dang chon.
      if (algo_ == gs::Algo::EXHAUSTIVE) hw::statusColor(25, 25, 25);
      else if (algo_ == gs::Algo::GRADIENT) hw::statusColor(0, 30, 30);
      else hw::statusColor(30, 0, 30);
      break;
    case Sys::RUN:
      hw::statusByLevel(io.gas().level);
      break;
    case Sys::DONE:
      if (now - blink_ms_ > 250) {
        blink_ms_ = now;
        blink_on_ = !blink_on_;
        hw::statusColor(0, blink_on_ ? 90 : 0, 0);  // xanh la nhap nhay
      }
      break;
  }
}

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  const uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 2000) {
  }

  hw::ioBegin();
  hw::statusColor(0, 0, 60);
  hw::gasBegin();
  hw::motorsBegin();

  hw::imuBegin();
  Serial.println("Do bias gyro - GIU YEN ROBOT...");
  hw::imuCalibrateBias(2000);

  hw::odomBegin();
  hw::motionBegin();
  radiolink::begin();

  io.begin();
  bench::begin(&io);
  banner();
  Serial.printf("Do baseline trong %lu ms (khong khi sach)...\n",
                (unsigned long)cfg::BASELINE_MS);

  next_control_ms_ = millis();
  next_tele_ms_ = millis() + 1000;
}

void loop() {
  const uint32_t now = millis();

  io.update();  // cam bien + odometry + chuyen dong
  hw::ioUpdate();
  radiolink::poll();
  pollSerial();
  pollLoRa();
  bench::update();

  // --- nut bam ---
  switch (hw::buttonPoll()) {
    case hw::BtnEvent::SHORT_PRESS:
      if (bench::active()) bench::abort();
      else if (sys_ == Sys::RUN) stopRun("nut");
      else startRun();
      break;
    case hw::BtnEvent::LONG_PRESS:
      if (sys_ != Sys::RUN) cycleAlgo();
      break;
    default:
      break;
  }

  // --- vong dieu khien ---
  if ((int32_t)(now - next_control_ms_) >= 0) {
    next_control_ms_ = now + cfg::CONTROL_PERIOD_MS;

    if (sys_ == Sys::CALIB && io.baselineReady()) {
      sys_ = Sys::READY;
      Serial.printf("Baseline = %u (ADC), R0 = %.0f ohm. San sang - nhan nut BOOT de chay.\n",
                    io.baseline(), io.r0());
      if (millis() < cfg::MQ3_PREHEAT_MS) {
        Serial.println("!! Canh bao: MQ-3 chua say du 5 phut, so lieu se troi.");
      }
      hw::beepPattern(2);
    }

    if (sys_ == Sys::RUN) {
      mission.update(io);
      if (!mission.running()) {
        hw::motionStop();
        io.setMotorsEnabled(false);
        sys_ = Sys::DONE;
        const gs::BestPoint b = mission.best();
        Serial.println("-----------------------------------------------------");
        Serial.printf("KET THUC: %s\n", mission.stateName());
        Serial.printf("  Thoi gian    : %.1f s\n", mission.elapsedMs(io) / 1000.0f);
        Serial.printf("  Quang duong  : %.0f cm\n", mission.pathCm(io));
        Serial.printf("  Diem cao nhat: (%.1f, %.1f) cm  norm=%d  ppm~%.0f\n", b.x_cm,
                      b.y_cm, b.normalized, b.ppm);
        Serial.println("-----------------------------------------------------");
        hw::beepPattern(3);
      }
    }
  }

  // --- telemetry ---
  if ((int32_t)(now - next_tele_ms_) >= 0) {
    next_tele_ms_ = now + cfg::TELEMETRY_PERIOD_MS;
    sendTelemetry();
  }

  updateLed();
}
