#include "robot_io.h"

#include "../core/config.h"
#include "hw_gas.h"
#include "hw_io.h"
#include "hw_motors.h"
#include "motion.h"
#include "odometry.h"

void RobotIO::begin() {
  next_gas_ms_ = millis();
  gas_.begin(millis());
  bump_latched_ = false;
}

void RobotIO::restartBaseline() { gas_.begin(millis()); }

void RobotIO::update() {
  const uint32_t now = millis();

  // --- cam bien khi theo chu ky co dinh ---
  if ((int32_t)(now - next_gas_ms_) >= 0) {
    next_gas_ms_ = now + cfg::GAS_SAMPLE_PERIOD_MS;
    last_adc_ = hw::gasReadRaw();
    last_mv_ = hw::gasReadMv();
    gas_.addSample(last_adc_, last_mv_, now);
  }

  // --- cham vao vat can: chot lai de thuat toan khong bo sot ---
  if (hw::bumperAny()) bump_latched_ = true;

  hw::odomUpdate();
  hw::motionUpdate();
}

gs::Pose RobotIO::pose() const { return hw::odomPose(); }
float RobotIO::travelledCm() const { return hw::odomTravelledCm(); }

void RobotIO::cmdForward(float cm) { hw::motionForward(cm); }
void RobotIO::cmdTurn(float delta_deg) { hw::motionTurn(delta_deg); }
void RobotIO::cmdStop() { hw::motionStop(); }
bool RobotIO::motionBusy() const { return hw::motionBusy(); }

bool RobotIO::bumped() const { return bump_latched_ || hw::bumperAny(); }
void RobotIO::clearBump() { bump_latched_ = false; }

void RobotIO::setMotorsEnabled(bool on) { hw::motorsEnable(on); }

void RobotIO::log(const char* msg) {
  Serial.print("[");
  Serial.print(millis() / 1000.0f, 1);
  Serial.print("s] ");
  Serial.println(msg);
}
