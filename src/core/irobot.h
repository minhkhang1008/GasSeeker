#pragma once
#include <cstdint>

#include "geometry.h"

namespace gs {

enum class AlarmLevel : uint8_t { Safe = 0, Detected = 1, High = 2, Critical = 3 };

const char* alarmLevelName(AlarmLevel lv);

struct GasReading {
  uint16_t raw = 0;
  int16_t normalized = 0;
  float ppm = 0.0f;
  AlarmLevel level = AlarmLevel::Safe;
  bool valid = false;
};

class IRobot {
 public:
  virtual ~IRobot() {}

  virtual uint32_t nowMs() const = 0;

  virtual GasReading gas() const = 0;
  virtual Pose pose() const = 0;
  virtual float travelledCm() const = 0;

  virtual void cmdForward(float cm) = 0;
  virtual void cmdTurn(float delta_deg) = 0;
  virtual void cmdStop() = 0;
  virtual bool motionBusy() const = 0;

  virtual bool bumped() const = 0;
  virtual void clearBump() = 0;

  virtual void log(const char* msg) = 0;
};

}
