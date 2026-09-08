#include "search_algorithm.h"

namespace gs {

void SurgeCastSearch::begin(IRobot& r) {
  seek_.begin(cfg::SEEK_STRIDE);
  best_.reset();
  bump_.reset();
  stop_.reset();
  nav_.abort(r);
  last_detect_ms_ = 0;
  cast_amp_ = cfg::SC_CAST_STEP_CM;
  cast_side_ = +1;
  cast_legs_ = 0;
  stall_.reset();
  r.log("SURGE_CAST: bat dau (SEEKING)");
  seekNext(r);
}

bool SurgeCastSearch::dirOk(const IRobot& r, float heading_deg, float step_cm) const {
  const Pose p = r.pose();
  float tx, ty;
  project(p.x_cm, p.y_cm, heading_deg, step_cm + cfg::SENSOR_OFFSET_CM, tx, ty);
  return insideArena(tx, ty);
}

void SurgeCastSearch::finish(IRobot& r) {
  const BestPoint& b = best_.get();
  if (cfg::RETURN_TO_BEST && b.valid) {
    const Pose p = r.pose();
    if (dist(p.x_cm, p.y_cm, b.x_cm, b.y_cm) > cfg::RETURN_MIN_DIST_CM) {
      r.log("SURGE_CAST: quay lai diem do cao nhat");
      nav_.goTo(r, b.x_cm, b.y_cm);
      ph_ = Ph::RETURN;
      return;
    }
  }
  r.cmdStop();
  ph_ = Ph::DONE;
}

void SurgeCastSearch::seekNext(IRobot& r) {
  float x, y;
  if (seek_.next(x, y)) {
    nav_.goTo(r, x, y);
    ph_ = Ph::SEEK_GOTO;
  } else {
    r.log("SURGE_CAST: quet het san van khong bat duoc luong -> dung");
    finish(r);
  }
}

void SurgeCastSearch::startSurge(IRobot& r) {
  const float upwind = cfg::WIND_FROM_DEG;
  if (!dirOk(r, upwind, cfg::SC_SURGE_STEP_CM)) {
    if (cast_legs_ == 0) cast_legs_ = 1;
    startCast(r);
    return;
  }
  const Pose p = r.pose();
  float tx, ty;
  project(p.x_cm, p.y_cm, upwind, cfg::SC_SURGE_STEP_CM, tx, ty);
  nav_.goTo(r, tx, ty);
  ph_ = Ph::SURGE_MOVE;
}

void SurgeCastSearch::startCast(IRobot& r) {
  if (cast_legs_ > 0) {
    cast_side_ = -cast_side_;
    cast_amp_ *= cfg::SC_CAST_GROWTH;
  }

  if (cast_amp_ > cfg::SC_CAST_MAX_CM) {
    r.log("SURGE_CAST: cast qua bien do gioi han -> quay ve SEEKING");
    cast_amp_ = cfg::SC_CAST_STEP_CM;
    cast_side_ = +1;
    cast_legs_ = 0;
    seekNext(r);
    return;
  }

  float heading = wrapDeg(cfg::WIND_FROM_DEG + cast_side_ * 90.0f);
  if (!dirOk(r, heading, cast_amp_)) {
    cast_side_ = -cast_side_;
    heading = wrapDeg(cfg::WIND_FROM_DEG + cast_side_ * 90.0f);
    if (!dirOk(r, heading, cast_amp_)) {
      cast_amp_ = cfg::SC_CAST_STEP_CM;
      cast_legs_ = 0;
      seekNext(r);
      return;
    }
  }

  const Pose p = r.pose();
  float tx, ty;
  project(p.x_cm, p.y_cm, heading, cast_amp_, tx, ty);
  nav_.goTo(r, tx, ty);
  ++cast_legs_;
  ph_ = Ph::CAST_MOVE;
}

void SurgeCastSearch::update(IRobot& r) {
  if (ph_ == Ph::DONE) return;

  if (bump_.active()) {
    if (bump_.update(r)) {
      if (ph_ == Ph::RETURN) {
        r.cmdStop();
        ph_ = Ph::DONE;
      } else if (ph_ == Ph::SEEK_GOTO || ph_ == Ph::SEEK_SNIFF) {
        seekNext(r);
      } else {
        startSurge(r);
      }
    }
    return;
  }
  if (bump_.triggerIfBumped(r)) {
    nav_.abort(r);
    return;
  }

  switch (ph_) {
    case Ph::SEEK_GOTO: {
      const bool arrived = !nav_.busy() || nav_.update(r);
      if (arrived) {
        r.cmdStop();
        sniff_.start(r.nowMs());
        ph_ = Ph::SEEK_SNIFF;
      }
      break;
    }

    case Ph::SEEK_SNIFF: {
      if (!sniff_.update(r)) break;
      best_.feed(r, sniff_.value(), sniff_.rawValue(), sniff_.ppm());
      if (sniff_.value() >= cfg::DETECT_DELTA) {
        last_detect_ms_ = r.nowMs();
        cast_amp_ = cfg::SC_CAST_STEP_CM;
        cast_side_ = +1;
        cast_legs_ = 0;
        r.log("SURGE_CAST: phat hien khi -> SURGE");
        startSurge(r);
      } else {
        seekNext(r);
      }
      break;
    }

    case Ph::SURGE_MOVE: {
      const bool arrived = !nav_.busy() || nav_.update(r);
      if (arrived) {
        r.cmdStop();
        sniff_.start(r.nowMs());
        ph_ = Ph::SURGE_SNIFF;
      }
      break;
    }

    case Ph::SURGE_SNIFF: {
      if (!sniff_.update(r)) break;
      const int16_t v = sniff_.value();
      best_.feed(r, v, sniff_.rawValue(), sniff_.ppm());
      const bool stalled = stall_.feed(v);

      if (stop_.feed(v, r.nowMs())) {
        r.log("SURGE_CAST: thoa dieu kien dung -> ket luan da toi gan nguon");
        finish(r);
        break;
      }
      if (stalled) {
        r.log("SURGE_CAST: qua lau khong cai thien -> ket luan bang diem cao nhat");
        finish(r);
        break;
      }

      if (v >= cfg::DETECT_DELTA) {
        last_detect_ms_ = r.nowMs();
        cast_amp_ = cfg::SC_CAST_STEP_CM;
        cast_side_ = +1;
        cast_legs_ = 0;
        startSurge(r);
      } else if (r.nowMs() - last_detect_ms_ >= cfg::SC_LOST_MS) {
        r.log("SURGE_CAST: mat tin hieu -> CAST");
        startCast(r);
      } else {
        startSurge(r);
      }
      break;
    }

    case Ph::CAST_MOVE: {
      const bool arrived = !nav_.busy() || nav_.update(r);
      if (arrived) {
        r.cmdStop();
        sniff_.start(r.nowMs());
        ph_ = Ph::CAST_SNIFF;
      }
      break;
    }

    case Ph::CAST_SNIFF: {
      if (!sniff_.update(r)) break;
      const int16_t v = sniff_.value();
      best_.feed(r, v, sniff_.rawValue(), sniff_.ppm());
      const bool stalled = stall_.feed(v);

      if (stop_.feed(v, r.nowMs())) {
        r.log("SURGE_CAST: thoa dieu kien dung -> ket luan da toi gan nguon");
        finish(r);
        break;
      }
      if (stalled) {
        r.log("SURGE_CAST: qua lau khong cai thien -> ket luan bang diem cao nhat");
        finish(r);
        break;
      }

      if (v >= cfg::DETECT_DELTA) {
        last_detect_ms_ = r.nowMs();
        cast_amp_ = cfg::SC_CAST_STEP_CM;
        cast_side_ = +1;
        cast_legs_ = 0;
        r.log("SURGE_CAST: bat lai duoc luong -> SURGE");
        startSurge(r);
      } else {
        startCast(r);
      }
      break;
    }

    case Ph::RETURN: {
      const bool arrived = !nav_.busy() || nav_.update(r);
      if (arrived) {
        r.cmdStop();
        ph_ = Ph::DONE;
      }
      break;
    }

    default:
      break;
  }
}

const char* SurgeCastSearch::stateName() const {
  switch (ph_) {
    case Ph::SEEK_GOTO:
    case Ph::SEEK_SNIFF: return "SEARCHING";
    case Ph::SURGE_MOVE:
    case Ph::SURGE_SNIFF: return "SURGE";
    case Ph::CAST_MOVE:
    case Ph::CAST_SNIFF: return (cast_side_ > 0) ? "CAST_LEFT" : "CAST_RIGHT";
    case Ph::RETURN: return "RETURN";
    case Ph::DONE: return "SOURCE_FOUND";
  }
  return "?";
}

}
