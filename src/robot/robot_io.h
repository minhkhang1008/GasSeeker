#pragma once
#include <Arduino.h>

#include "../core/gas.h"
#include "../core/irobot.h"

class RobotIO : public gs::IRobot {
 public:
  void begin();
  void update();

  void restartBaseline();
  bool baselineReady() const { return gas_.baselineReady(); }
  uint16_t baseline() const { return gas_.baseline(); }
  float r0() const { return gas_.r0(); }
  float rs() const { return gas_.rsNow(); }
  uint16_t lastAdc() const { return last_adc_; }
  float lastMv() const { return last_mv_; }

  void setMotorsEnabled(bool on);

  uint32_t nowMs() const override { return millis(); }
  gs::GasReading gas() const override { return gas_.reading(); }
  gs::Pose pose() const override;
  float travelledCm() const override;
  void cmdForward(float cm) override;
  void cmdTurn(float delta_deg) override;
  void cmdStop() override;
  bool motionBusy() const override;
  bool bumped() const override;
  void clearBump() override;
  void log(const char* msg) override;

 private:
  gs::GasProcessor gas_;
  uint32_t next_gas_ms_ = 0;
  uint32_t next_odom_us_ = 0;
  uint16_t last_adc_ = 0;
  float last_mv_ = 0.0f;
  mutable bool bump_latched_ = false;
};
