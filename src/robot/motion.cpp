#include "motion.h"

#include "../core/config.h"
#include "../core/geometry.h"
#include "hw_motors.h"
#include "odometry.h"

namespace hw {

enum class M : uint8_t { IDLE, DRIVE, TURN, BRAKE };

static M mode_ = M::IDLE;
static float target_cm_ = 0.0f;
static float target_heading_ = 0.0f;
static float drive_sign_ = 1.0f;
static uint32_t t_start_ = 0;
static uint32_t brake_until_ = 0;
static uint32_t settle_since_ = 0;

void motionBegin() {
  mode_ = M::IDLE;
  motorsSet(0, 0);
  odomSetWheelDir(0, 0);
}

static void enterBrake() {
  motorsBrake();
  odomSetWheelDir(0, 0);
  mode_ = M::BRAKE;
  brake_until_ = millis() + cfg::BRAKE_MS;
}

void motionForward(float cm) {
  if (fabsf(cm) < 0.5f) {
    enterBrake();
    return;
  }
  drive_sign_ = (cm > 0) ? 1.0f : -1.0f;
  target_cm_ = fabsf(cm);
  target_heading_ = odomPose().heading_deg;
  odomSegmentReset();
  t_start_ = millis();
  mode_ = M::DRIVE;
}

void motionTurn(float delta_deg) {
  const float d = gs::wrapDeg(delta_deg);
  if (fabsf(d) < 0.5f) {
    enterBrake();
    return;
  }
  target_heading_ = gs::wrapDeg(odomPose().heading_deg + d);
  odomSegmentReset();
  settle_since_ = 0;
  t_start_ = millis();
  mode_ = M::TURN;
}

void motionStop() {
  if (mode_ != M::IDLE) enterBrake();
  else motorsSet(0, 0);
}

bool motionBusy() { return mode_ != M::IDLE; }

void motionUpdate() {
  const uint32_t now = millis();

  switch (mode_) {
    case M::DRIVE: {
      if (odomSegmentCm() >= target_cm_ || now - t_start_ > cfg::MOTION_TIMEOUT_MS) {
        enterBrake();
        break;
      }
      // Giu huong: sai so duong -> can quay trai -> banh trai cham lai.
      const float err = gs::wrapDeg(target_heading_ - odomPose().heading_deg);
      int corr = (int)(cfg::HEADING_KP * err);
      corr = gs::clampv(corr, -cfg::HEADING_MAX_CORR, cfg::HEADING_MAX_CORR);

      const int base = (int)(cfg::PWM_DRIVE * drive_sign_);
      // Khi lui, dau cua bu huong cung phai dao lai.
      const int c = (drive_sign_ > 0) ? corr : -corr;
      motorsSet(base - c, base + c);
      odomSetWheelDir((int)drive_sign_, (int)drive_sign_);
      break;
    }

    case M::TURN: {
      const float err = gs::wrapDeg(target_heading_ - odomPose().heading_deg);
      if (fabsf(err) <= cfg::TURN_TOLERANCE_DEG) {
        if (settle_since_ == 0) settle_since_ = now;
        // Phai giu trong vung sai so mot chut moi coi la quay xong.
        if (now - settle_since_ >= 120) {
          enterBrake();
          break;
        }
        motorsSet(0, 0);
        odomSetWheelDir(0, 0);
        break;
      }
      settle_since_ = 0;
      if (now - t_start_ > cfg::MOTION_TIMEOUT_MS) {
        enterBrake();
        break;
      }
      int pwm = (int)(cfg::TURN_KP * err);
      pwm = gs::clampv(pwm, -cfg::PWM_TURN, cfg::PWM_TURN);
      // err > 0 -> quay trai (CCW): banh trai lui, banh phai tien.
      motorsSet(-pwm, pwm);
      odomSetWheelDir(pwm > 0 ? -1 : 1, pwm > 0 ? 1 : -1);
      break;
    }

    case M::BRAKE:
      if (now >= brake_until_) {
        motorsSet(0, 0);
        odomSetWheelDir(0, 0);
        mode_ = M::IDLE;
      }
      break;

    case M::IDLE:
    default:
      odomSetWheelDir(0, 0);
      break;
  }
}

const char* motionStateName() {
  switch (mode_) {
    case M::IDLE: return "IDLE";
    case M::DRIVE: return "DRIVE";
    case M::TURN: return "TURN";
    case M::BRAKE: return "BRAKE";
  }
  return "?";
}

}  // namespace hw
