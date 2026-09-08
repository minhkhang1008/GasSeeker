#pragma once
#include <cstdint>

#include "config.h"
#include "gas.h"
#include "geometry.h"
#include "irobot.h"

namespace gs {

struct BestPoint {
  float x_cm = 0.0f;
  float y_cm = 0.0f;
  int16_t normalized = -32768;
  uint16_t raw = 0;
  float ppm = 0.0f;
  uint32_t t_ms = 0;
  bool valid = false;
};

class BestTracker {
 public:
  void reset() { b_ = BestPoint{}; }
  bool feed(const IRobot& r, int16_t normalized, uint16_t raw, float ppm);
  const BestPoint& get() const { return b_; }

 private:
  BestPoint b_;
};

class Navigator {
 public:
  void goTo(IRobot& r, float x_cm, float y_cm);
  void turnTo(IRobot& r, float heading_deg);
  void turnBy(IRobot& r, float delta_deg);
  void forward(IRobot& r, float cm);
  void abort(IRobot& r);

  bool update(IRobot& r);
  bool busy() const { return ph_ != Ph::IDLE; }

 private:
  enum class Ph : uint8_t { IDLE, TURN, DRIVE };
  Ph ph_ = Ph::IDLE;
  float pending_drive_cm_ = 0.0f;
};

class StopDetector {
 public:
  void reset();
  bool feed(int16_t normalized, uint32_t now_ms);
  int16_t best() const { return best_; }
  bool improvedLast() const { return improved_; }

 private:
  int16_t best_ = -32768;
  bool holding_ = false;
  bool improved_ = false;
  uint32_t hold_start_ = 0;
};

class StallGuard {
 public:
  void reset() {
    ref_ = -32768;
    n_ = 0;
  }
  bool feed(int16_t normalized);
  int count() const { return n_; }

 private:
  int16_t ref_ = -32768;
  int n_ = 0;
};

class BumpRecovery {
 public:
  void reset() { ph_ = Ph::IDLE; }
  bool triggerIfBumped(IRobot& r);
  bool active() const { return ph_ != Ph::IDLE; }
  bool update(IRobot& r);

 private:
  enum class Ph : uint8_t { IDLE, BACKING, TURNING };
  Ph ph_ = Ph::IDLE;
  int side_ = 1;
};

class LawnmowerPath {
 public:
  void begin(int stride);
  bool next(float& x_cm, float& y_cm);
  bool done() const { return done_; }
  int index() const { return k_; }
  int total() const { return total_; }

 private:
  int stride_ = 1;
  int k_ = 0;
  int nx_ = 0, ny_ = 0;
  int total_ = 0;
  bool done_ = false;
};

}
