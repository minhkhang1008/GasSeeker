#include "odometry.h"

#include "../core/config.h"
#include "hw_imu.h"

namespace hw {

// --- bo dem xung tu ngat ---
static volatile uint32_t tick_l_ = 0;
static volatile uint32_t tick_r_ = 0;
static volatile uint32_t last_us_l_ = 0;
static volatile uint32_t last_us_r_ = 0;
// Encoder 20 khe, banh 65 mm: o toc do lam viec moi xung cach nhau > 20 ms.
// Chan cac xung duoi 1.5 ms -> loai nhieu do rung va nhieu dien tu tu motor.
static constexpr uint32_t TICK_MIN_US = 1500;

static void IRAM_ATTR isrL() {
  const uint32_t now = micros();
  if (now - last_us_l_ < TICK_MIN_US) return;
  last_us_l_ = now;
  ++tick_l_;
}

static void IRAM_ATTR isrR() {
  const uint32_t now = micros();
  if (now - last_us_r_ < TICK_MIN_US) return;
  last_us_r_ = now;
  ++tick_r_;
}

// --- trang thai ---
static uint32_t prev_l_ = 0, prev_r_ = 0;
static int dir_l_ = 0, dir_r_ = 0;
static uint32_t last_update_us_ = 0;

static float x_cm_ = 0, y_cm_ = 0, heading_deg_ = 0;
static float travelled_cm_ = 0;
static float seg_cm_ = 0, seg_turn_deg_ = 0;
static bool use_gyro_ = false;

static constexpr float WHEEL_BASE_CM = cfg::WHEEL_BASE_MM / 10.0f;

void odomBegin() {
  pinMode(cfg::pin::ENC_L, INPUT_PULLUP);
  pinMode(cfg::pin::ENC_R, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(cfg::pin::ENC_L), isrL, RISING);
  attachInterrupt(digitalPinToInterrupt(cfg::pin::ENC_R), isrR, RISING);

  use_gyro_ = imuOk();
  last_update_us_ = micros();
  odomReset(cfg::START_X_CM, cfg::START_Y_CM, cfg::START_HEADING_DEG);
}

void odomReset(float x_cm, float y_cm, float heading_deg) {
  noInterrupts();
  prev_l_ = tick_l_;
  prev_r_ = tick_r_;
  interrupts();
  x_cm_ = x_cm;
  y_cm_ = y_cm;
  heading_deg_ = heading_deg;
  travelled_cm_ = 0.0f;
  seg_cm_ = 0.0f;
  seg_turn_deg_ = 0.0f;
  last_update_us_ = micros();
}

void odomSetWheelDir(int left_dir, int right_dir) {
  dir_l_ = left_dir;
  dir_r_ = right_dir;
}

bool odomUsingGyro() { return use_gyro_; }

void odomUpdate() {
  const uint32_t now_us = micros();
  float dt = (now_us - last_update_us_) * 1e-6f;
  last_update_us_ = now_us;
  if (dt <= 0.0f || dt > 0.5f) dt = 0.0f;  // bo qua buoc bat thuong

  noInterrupts();
  const uint32_t tl = tick_l_;
  const uint32_t tr = tick_r_;
  interrupts();

  const long dl_ticks = (long)(tl - prev_l_);
  const long dr_ticks = (long)(tr - prev_r_);
  prev_l_ = tl;
  prev_r_ = tr;

  // Chieu quay suy ra tu lenh dang cap cho motor.
  const float dl = dl_ticks * cfg::CM_PER_TICK * (float)dir_l_;
  const float dr = dr_ticks * cfg::CM_PER_TICK * (float)dir_r_;
  const float d = 0.5f * (dl + dr);

  // --- huong ---
  float dtheta;
  if (use_gyro_) {
    dtheta = imuGyroZ() * dt;
  } else {
    dtheta = gs::rad2deg((dr - dl) / WHEEL_BASE_CM);
  }
  heading_deg_ = gs::wrapDeg(heading_deg_ + dtheta);
  seg_turn_deg_ += dtheta;

  // --- vi tri ---
  x_cm_ += d * cosf(gs::deg2rad(heading_deg_));
  y_cm_ += d * sinf(gs::deg2rad(heading_deg_));
  travelled_cm_ += fabsf(d);
  seg_cm_ += fabsf(d);
}

gs::Pose odomPose() {
  gs::Pose p;
  p.x_cm = x_cm_;
  p.y_cm = y_cm_;
  p.heading_deg = heading_deg_;
  return p;
}

float odomTravelledCm() { return travelled_cm_; }

void odomSegmentReset() {
  seg_cm_ = 0.0f;
  seg_turn_deg_ = 0.0f;
}

float odomSegmentCm() { return seg_cm_; }
float odomSegmentTurnDeg() { return seg_turn_deg_; }

long odomTicksL() { return (long)tick_l_; }
long odomTicksR() { return (long)tick_r_; }

}  // namespace hw
