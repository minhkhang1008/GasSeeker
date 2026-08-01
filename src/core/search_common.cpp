#include "search_common.h"

#include <cmath>

namespace gs {

// ---------------------------------------------------------------------------
bool BestTracker::feed(const IRobot& r, int16_t normalized, uint16_t raw, float ppm) {
  if (b_.valid && normalized <= b_.normalized) return false;
  const Pose p = r.pose();
  // Ghi lai vi tri CUA DAU DO chu khong phai tam xe.
  float sx, sy;
  project(p.x_cm, p.y_cm, p.heading_deg, cfg::SENSOR_OFFSET_CM, sx, sy);
  b_.x_cm = sx;
  b_.y_cm = sy;
  b_.normalized = normalized;
  b_.raw = raw;
  b_.ppm = ppm;
  b_.t_ms = r.nowMs();
  b_.valid = true;
  return true;
}

// ---------------------------------------------------------------------------
void Navigator::goTo(IRobot& r, float x_cm, float y_cm) {
  const Pose p = r.pose();
  const float d = dist(p.x_cm, p.y_cm, x_cm, y_cm);
  if (d < 2.0f) {  // da o day roi
    ph_ = Ph::IDLE;
    pending_drive_cm_ = 0.0f;
    r.cmdStop();
    return;
  }
  const float brg = bearingDeg(p.x_cm, p.y_cm, x_cm, y_cm);
  pending_drive_cm_ = d;
  ph_ = Ph::TURN;
  r.cmdTurn(wrapDeg(brg - p.heading_deg));
}

void Navigator::turnTo(IRobot& r, float heading_deg) {
  turnBy(r, wrapDeg(heading_deg - r.pose().heading_deg));
}

void Navigator::turnBy(IRobot& r, float delta_deg) {
  pending_drive_cm_ = 0.0f;
  ph_ = Ph::TURN;
  r.cmdTurn(delta_deg);
}

void Navigator::forward(IRobot& r, float cm) {
  pending_drive_cm_ = 0.0f;
  ph_ = Ph::DRIVE;
  r.cmdForward(cm);
}

void Navigator::abort(IRobot& r) {
  ph_ = Ph::IDLE;
  pending_drive_cm_ = 0.0f;
  r.cmdStop();
}

bool Navigator::update(IRobot& r) {
  if (ph_ == Ph::IDLE) return false;
  if (r.motionBusy()) return false;

  if (ph_ == Ph::TURN) {
    if (pending_drive_cm_ > 0.0f) {
      const float d = pending_drive_cm_;
      pending_drive_cm_ = 0.0f;
      ph_ = Ph::DRIVE;
      r.cmdForward(d);
      return false;
    }
    ph_ = Ph::IDLE;
    return true;
  }
  // Ph::DRIVE
  ph_ = Ph::IDLE;
  return true;
}

// ---------------------------------------------------------------------------
void StopDetector::reset() {
  best_ = -32768;
  holding_ = false;
  improved_ = false;
  hold_start_ = 0;
}

bool StopDetector::feed(int16_t normalized, uint32_t now_ms) {
  improved_ = false;

  // (b) "con tang dang ke" = vuot ky luc cu qua PLATEAU_EPS.
  const bool significant_gain =
      (best_ == -32768) || (normalized > best_ + cfg::PLATEAU_EPS);
  if (significant_gain) {
    improved_ = true;
    holding_ = false;  // con tang -> chua duoc dung
  }
  if (normalized > best_) best_ = normalized;

  // (a) du cao
  if (normalized < cfg::STOP_HIGH_DELTA) {
    holding_ = false;
    return false;
  }
  if (significant_gain) return false;

  // (c) duy tri du lau
  if (!holding_) {
    holding_ = true;
    hold_start_ = now_ms;
    return false;
  }
  return (now_ms - hold_start_) >= cfg::STOP_HOLD_MS;
}

// ---------------------------------------------------------------------------
bool StallGuard::feed(int16_t v) {
  if (v > ref_ + cfg::PLATEAU_EPS) {
    ref_ = v;
    n_ = 0;
  } else {
    ++n_;
  }
  return n_ >= cfg::STALL_LIMIT_SNIFFS;
}

// ---------------------------------------------------------------------------
bool BumpRecovery::triggerIfBumped(IRobot& r) {
  if (ph_ != Ph::IDLE) return false;
  if (!r.bumped()) return false;
  r.log("BUMP -> lui lai va doi huong");
  r.clearBump();
  ph_ = Ph::BACKING;
  r.cmdForward(-12.0f);
  return true;
}

bool BumpRecovery::update(IRobot& r) {
  if (ph_ == Ph::IDLE) return false;
  if (r.motionBusy()) return false;

  if (ph_ == Ph::BACKING) {
    ph_ = Ph::TURNING;
    side_ = -side_;
    r.cmdTurn(side_ * 75.0f);
    return false;
  }
  ph_ = Ph::IDLE;
  return true;
}

// ---------------------------------------------------------------------------
void LawnmowerPath::begin(int stride) {
  stride_ = stride < 1 ? 1 : stride;
  k_ = 0;
  done_ = false;
  nx_ = (cfg::GRID_NX + stride_ - 1) / stride_;
  ny_ = (cfg::GRID_NY + stride_ - 1) / stride_;
  total_ = nx_ * ny_;
}

bool LawnmowerPath::next(float& x_cm, float& y_cm) {
  if (k_ >= total_) {
    done_ = true;
    return false;
  }
  const int row = k_ / nx_;
  int col = k_ % nx_;
  if (row % 2 == 1) col = nx_ - 1 - col;  // hang le di nguoc lai -> zig-zag

  const int ci = col * stride_;
  const int cj = row * stride_;
  x_cm = (ci + 0.5f) * cfg::CELL_CM;
  y_cm = (cj + 0.5f) * cfg::CELL_CM;

  // Khong bao gio nham diem den ra ngoai le an toan.
  x_cm = clampv(x_cm, cfg::ARENA_MARGIN_CM, cfg::ARENA_W_CM - cfg::ARENA_MARGIN_CM);
  y_cm = clampv(y_cm, cfg::ARENA_MARGIN_CM, cfg::ARENA_H_CM - cfg::ARENA_MARGIN_CM);

  ++k_;
  return true;
}

}  // namespace gs
