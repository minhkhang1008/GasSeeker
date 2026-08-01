#include "hw_motors.h"

#include "../core/config.h"

namespace hw {

static constexpr int PWM_CH_L = 0;
static constexpr int PWM_CH_R = 1;
static constexpr int PWM_FREQ_HZ = 20000;  // ngoai nguong nghe -> motor khong ru
static constexpr int PWM_RES_BITS = 8;

static bool enabled_ = false;

void motorsBegin() {
  pinMode(cfg::pin::MOT_L_IN1, OUTPUT);
  pinMode(cfg::pin::MOT_L_IN2, OUTPUT);
  pinMode(cfg::pin::MOT_R_IN1, OUTPUT);
  pinMode(cfg::pin::MOT_R_IN2, OUTPUT);
  pinMode(cfg::pin::MOT_STBY, OUTPUT);

  ledcSetup(PWM_CH_L, PWM_FREQ_HZ, PWM_RES_BITS);
  ledcSetup(PWM_CH_R, PWM_FREQ_HZ, PWM_RES_BITS);
  ledcAttachPin(cfg::pin::MOT_L_PWM, PWM_CH_L);
  ledcAttachPin(cfg::pin::MOT_R_PWM, PWM_CH_R);

  motorsCoast();
  motorsEnable(false);
}

void motorsEnable(bool on) {
  enabled_ = on;
  digitalWrite(cfg::pin::MOT_STBY, on ? HIGH : LOW);
  if (!on) motorsCoast();
}

bool motorsEnabled() { return enabled_; }

// Bu vung chet: duoi PWM_MIN_MOVE motor khong quay ma chi keu va nong.
static int applyDeadband(int v) {
  if (v == 0) return 0;
  const int mag = abs(v);
  const int out = (mag < cfg::PWM_MIN_MOVE) ? cfg::PWM_MIN_MOVE : mag;
  return (v > 0) ? min(out, cfg::PWM_MAX) : -min(out, cfg::PWM_MAX);
}

static void driveOne(int in1, int in2, int ch, int v) {
  v = applyDeadband(v);
  if (v > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    ledcWrite(ch, v);
  } else if (v < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    ledcWrite(ch, -v);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    ledcWrite(ch, 0);
  }
}

void motorsSet(int left, int right) {
  if (!enabled_) {
    motorsCoast();
    return;
  }
  driveOne(cfg::pin::MOT_L_IN1, cfg::pin::MOT_L_IN2, PWM_CH_L, left);
  driveOne(cfg::pin::MOT_R_IN1, cfg::pin::MOT_R_IN2, PWM_CH_R, right);
}

void motorsBrake() {
  digitalWrite(cfg::pin::MOT_L_IN1, HIGH);
  digitalWrite(cfg::pin::MOT_L_IN2, HIGH);
  digitalWrite(cfg::pin::MOT_R_IN1, HIGH);
  digitalWrite(cfg::pin::MOT_R_IN2, HIGH);
  ledcWrite(PWM_CH_L, 0);
  ledcWrite(PWM_CH_R, 0);
}

void motorsCoast() {
  digitalWrite(cfg::pin::MOT_L_IN1, LOW);
  digitalWrite(cfg::pin::MOT_L_IN2, LOW);
  digitalWrite(cfg::pin::MOT_R_IN1, LOW);
  digitalWrite(cfg::pin::MOT_R_IN2, LOW);
  ledcWrite(PWM_CH_L, 0);
  ledcWrite(PWM_CH_R, 0);
}

}  // namespace hw
