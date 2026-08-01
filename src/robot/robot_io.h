// ============================================================================
//  robot_io.h - hien thuc IRobot bang phan cung that.
//
//  Day la lop DUY NHAT noi giua thuat toan (src/core) va phan cung (src/robot).
//  Neu doi phan cung, chi phai sua file nay - khong dong den thuat toan.
// ============================================================================
#pragma once
#include <Arduino.h>

#include "../core/gas.h"
#include "../core/irobot.h"

class RobotIO : public gs::IRobot {
 public:
  void begin();
  // Goi moi vong lap: lay mau khi theo chu ky, cap nhat odometry va chuyen dong.
  void update();

  // Bat dau lai giai doan do baseline (khong khi sach).
  void restartBaseline();
  bool baselineReady() const { return gas_.baselineReady(); }
  uint16_t baseline() const { return gas_.baseline(); }
  float r0() const { return gas_.r0(); }
  float rs() const { return gas_.rsNow(); }
  uint16_t lastAdc() const { return last_adc_; }
  float lastMv() const { return last_mv_; }

  void setMotorsEnabled(bool on);

  // --- IRobot ---
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
