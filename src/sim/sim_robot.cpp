#include "sim_robot.h"

#include <cmath>
#include <cstdio>

#include "../core/config.h"
#include "../core/geometry.h"

namespace sim {

using gs::deg2rad;
using gs::wrapDeg;

// Nua chieu dai than xe: cham tuong khi tam xe toi gan mep hon muc nay.
static constexpr float BODY_HALF_CM = 11.0f;

void SimRobot::begin(Env env, float src_x, float src_y, uint32_t seed) {
  rng_.seed(seed * 2654435761u + 12345u);
  plume_.begin(env, src_x, src_y, cfg::WIND_FROM_DEG, seed);
  mq3_.begin(seed + 7777u);

  true_ = gs::Pose{cfg::START_X_CM, cfg::START_Y_CM, cfg::START_HEADING_DEG};
  odom_ = true_;
  travelled_cm_ = 0.0f;
  bumped_ = false;
  mode_ = Mode::IDLE;
  remain_cm_ = remain_deg_ = 0.0f;
  t_ms_ = 0;
  t_acc_s_ = 0.0;
  next_gas_ms_ = 0;

  std::uniform_real_distribution<float> u(-1.0f, 1.0f);
  dist_scale_err_ = u(rng_) * ODO_DIST_SCALE_ERR;
  heading_drift_dps_ = u(rng_) * ODO_HEADING_DRIFT_DPS;

  gasproc_.begin(0);
}

void SimRobot::warmup(float seconds) {
  const int n = (int)(seconds / DT_S);
  for (int i = 0; i < n; ++i) step(DT_S);
}

void SimRobot::cmdForward(float cm) {
  if (std::fabs(cm) < 0.5f) {
    cmdStop();
    return;
  }
  mode_ = Mode::DRIVE;
  dir_ = cm > 0 ? 1.0f : -1.0f;
  remain_cm_ = std::fabs(cm);
}

void SimRobot::cmdTurn(float delta_deg) {
  const float d = wrapDeg(delta_deg);
  if (std::fabs(d) < 0.5f) {
    cmdStop();
    return;
  }
  mode_ = Mode::TURN;
  dir_ = d > 0 ? 1.0f : -1.0f;
  remain_deg_ = std::fabs(d);
}

void SimRobot::cmdStop() {
  mode_ = Mode::IDLE;
  remain_cm_ = remain_deg_ = 0.0f;
}

void SimRobot::log(const char* msg) {
  if (verbose_) std::printf("      [%7.2fs] %s\n", t_ms_ / 1000.0f, msg);
}

void SimRobot::step(float dt_s) {
  // ---- thoi gian ----
  t_acc_s_ += dt_s;
  t_ms_ = (uint32_t)(t_acc_s_ * 1000.0 + 0.5);

  // ---- moi truong ----
  plume_.step(dt_s);

  // ---- chuyen dong ----
  switch (mode_) {
    case Mode::DRIVE: {
      float d = DRIVE_SPEED_CMS * dt_s;
      if (d > remain_cm_) d = remain_cm_;
      remain_cm_ -= d;

      // Gyro bi troi -> xe di hoi cong ma khong biet.
      true_.heading_deg = wrapDeg(true_.heading_deg + heading_drift_dps_ * dt_s);

      const float dd = d * dir_;
      true_.x_cm += dd * std::cos(deg2rad(true_.heading_deg));
      true_.y_cm += dd * std::sin(deg2rad(true_.heading_deg));

      // Odometry: robot tin rang no di dung doan da lenh, theo huong no tin.
      const float dd_odo = dd * (1.0f + dist_scale_err_);
      odom_.x_cm += dd_odo * std::cos(deg2rad(odom_.heading_deg));
      odom_.y_cm += dd_odo * std::sin(deg2rad(odom_.heading_deg));

      travelled_cm_ += d;

      // Va cham tuong.
      if (true_.x_cm < BODY_HALF_CM || true_.x_cm > cfg::ARENA_W_CM - BODY_HALF_CM ||
          true_.y_cm < BODY_HALF_CM || true_.y_cm > cfg::ARENA_H_CM - BODY_HALF_CM) {
        true_.x_cm = gs::clampv(true_.x_cm, BODY_HALF_CM, cfg::ARENA_W_CM - BODY_HALF_CM);
        true_.y_cm = gs::clampv(true_.y_cm, BODY_HALF_CM, cfg::ARENA_H_CM - BODY_HALF_CM);
        bumped_ = true;
        mode_ = Mode::BRAKE;
        brake_until_ms_ = t_ms_ + cfg::BRAKE_MS;
        remain_cm_ = 0.0f;
        break;
      }

      if (remain_cm_ <= 0.0f) {
        mode_ = Mode::BRAKE;
        brake_until_ms_ = t_ms_ + cfg::BRAKE_MS;
      }
      break;
    }

    case Mode::TURN: {
      float a = TURN_RATE_DPS * dt_s;
      if (a > remain_deg_) a = remain_deg_;
      remain_deg_ -= a;

      const float da = a * dir_;
      true_.heading_deg = wrapDeg(true_.heading_deg + da);
      odom_.heading_deg = wrapDeg(odom_.heading_deg + da);

      if (remain_deg_ <= 0.0f) {
        // Sai so con lai sau moi lan quay (truot banh, quan tinh).
        true_.heading_deg = wrapDeg(true_.heading_deg + gauss_(rng_) * ODO_TURN_ERR_DEG);
        mode_ = Mode::BRAKE;
        brake_until_ms_ = t_ms_ + cfg::BRAKE_MS;
      }
      break;
    }

    case Mode::BRAKE:
      if (t_ms_ >= brake_until_ms_) mode_ = Mode::IDLE;
      break;

    case Mode::IDLE:
    default:
      break;
  }

  // ---- cam bien khi ----
  sampleGas(dt_s);
}

void SimRobot::sampleGas(float dt_s) {
  // Dau do gan o dau xe, khong phai tam truc.
  float sx, sy;
  gs::project(true_.x_cm, true_.y_cm, true_.heading_deg, cfg::SENSOR_OFFSET_CM, sx, sy);
  const float c = plume_.concentration(sx, sy);

  // Mo hinh tre cua MQ-3 chay theo tung buoc mo phong...
  const uint16_t adc = mq3_.sample(c, dt_s);
  last_adc_ = adc;

  // ...nhung firmware chi lay mau theo chu ky cua no.
  if (t_ms_ >= next_gas_ms_) {
    next_gas_ms_ = t_ms_ + cfg::GAS_SAMPLE_PERIOD_MS;
    gasproc_.addSample(adc, 0.0f, t_ms_);
  }
}

}  // namespace sim
