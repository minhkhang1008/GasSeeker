#include "search_algorithm.h"

namespace gs {

void GradientSearch::begin(IRobot& r) {
  seek_.begin(cfg::SEEK_STRIDE);
  best_.reset();
  bump_.reset();
  stop_.reset();
  nav_.abort(r);
  acquired_ = false;
  no_gain_steps_ = 0;
  stall_.reset();
  prev_ = -32768;
  gC_ = gL_ = gR_ = 0;
  r.log("GRADIENT: bat dau (SEEK - quet tho de bat luong khi)");
  seekNext(r);
}

void GradientSearch::seekNext(IRobot& r) {
  float x, y;
  if (seek_.next(x, y)) {
    nav_.goTo(r, x, y);
    ph_ = Ph::SEEK_GOTO;
  } else {
    r.log("GRADIENT: quet het san van khong phat hien khi -> dung");
    finish(r);
  }
}

bool GradientSearch::dirOk(const IRobot& r, float heading_deg, float step_cm) const {
  const Pose p = r.pose();
  float tx, ty;
  project(p.x_cm, p.y_cm, heading_deg, step_cm + cfg::SENSOR_OFFSET_CM, tx, ty);
  return insideArena(tx, ty);
}

static bool feedAndCheckStall(BestTracker& best, StallGuard& stall, Sniffer& sn, IRobot& r) {
  best.feed(r, sn.value(), sn.rawValue(), sn.ppm());
  return stall.feed(sn.value());
}

void GradientSearch::stepForward(IRobot& r, float heading_deg, float step_cm) {
  const Pose p = r.pose();
  float tx, ty;
  project(p.x_cm, p.y_cm, heading_deg, step_cm, tx, ty);
  nav_.goTo(r, tx, ty);
  ph_ = Ph::STEP;
}

void GradientSearch::startSweep(IRobot& r) {
  base_heading_ = r.pose().heading_deg;
  nav_.turnBy(r, +cfg::GRAD_SWEEP_DEG);
  ph_ = Ph::SWEEP_TURN_L;
}

void GradientSearch::finish(IRobot& r) {
  const BestPoint& b = best_.get();
  if (cfg::RETURN_TO_BEST && b.valid) {
    const Pose p = r.pose();
    if (dist(p.x_cm, p.y_cm, b.x_cm, b.y_cm) > cfg::RETURN_MIN_DIST_CM) {
      r.log("GRADIENT: quay lai diem do cao nhat");
      nav_.goTo(r, b.x_cm, b.y_cm);
      ph_ = Ph::RETURN;
      return;
    }
  }
  r.cmdStop();
  ph_ = Ph::DONE;
}

void GradientSearch::update(IRobot& r) {
  if (ph_ == Ph::DONE) return;

  if (bump_.active()) {
    if (bump_.update(r)) {
      if (ph_ == Ph::RETURN) {
        r.cmdStop();
        ph_ = Ph::DONE;
      } else if (!acquired_) {
        seekNext(r);
      } else {
        startSweep(r);
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
        acquired_ = true;
        prev_ = sniff_.value();
        no_gain_steps_ = 0;
        r.log("GRADIENT: bat duoc luong khi -> chuyen sang bam gradient");
        startSweep(r);
      } else {
        seekNext(r);
      }
      break;
    }

    case Ph::STEP: {
      const bool arrived = !nav_.busy() || nav_.update(r);
      if (arrived) {
        r.cmdStop();
        sniff_.start(r.nowMs());
        ph_ = Ph::STEP_SNIFF;
      }
      break;
    }

    case Ph::STEP_SNIFF: {
      if (!sniff_.update(r)) break;
      const int16_t v = sniff_.value();
      const bool stalled = feedAndCheckStall(best_, stall_, sniff_, r);

      if (stop_.feed(v, r.nowMs())) {
        r.log("GRADIENT: thoa dieu kien dung -> ket luan da toi gan nguon");
        finish(r);
        break;
      }
      if (stalled) {
        r.log("GRADIENT: qua lau khong cai thien -> ket luan bang diem cao nhat");
        finish(r);
        break;
      }

      const bool rising = (v > prev_ + cfg::PLATEAU_EPS);
      prev_ = v;

      if (rising) {
        no_gain_steps_ = 0;
        const float h = r.pose().heading_deg;
        if (dirOk(r, h, cfg::GRAD_STEP_CM)) {
          stepForward(r, h, cfg::GRAD_STEP_CM);
          break;
        }
      }
      ++no_gain_steps_;
      startSweep(r);
      break;
    }

    case Ph::SWEEP_TURN_L: {
      if (!nav_.update(r)) break;
      sniff_.start(r.nowMs());
      ph_ = Ph::SWEEP_SNIFF_L;
      break;
    }

    case Ph::SWEEP_SNIFF_L: {
      if (!sniff_.update(r)) break;
      gL_ = sniff_.value();
      feedAndCheckStall(best_, stall_, sniff_, r);
      nav_.turnBy(r, -2.0f * cfg::GRAD_SWEEP_DEG);
      ph_ = Ph::SWEEP_TURN_R;
      break;
    }

    case Ph::SWEEP_TURN_R: {
      if (!nav_.update(r)) break;
      sniff_.start(r.nowMs());
      ph_ = Ph::SWEEP_SNIFF_R;
      break;
    }

    case Ph::SWEEP_SNIFF_R: {
      if (!sniff_.update(r)) break;
      gR_ = sniff_.value();
      feedAndCheckStall(best_, stall_, sniff_, r);
      gC_ = prev_;
      ph_ = Ph::SWEEP_DECIDE;
      break;
    }

    case Ph::SWEEP_DECIDE: {
      float step = cfg::GRAD_STEP_CM;
      float chosen = base_heading_;

      if (no_gain_steps_ >= cfg::GRAD_STUCK_STEPS) {
        r.log("GRADIENT: khong cai thien lau -> buoc thoat cuc tri");
        no_gain_steps_ = 0;
        step = cfg::GRAD_STEP_CM * 1.5f;
        const float e1 = wrapDeg(base_heading_ + cfg::GRAD_ESCAPE_DEG);
        const float e2 = wrapDeg(base_heading_ - cfg::GRAD_ESCAPE_DEG);
        if (dirOk(r, e1, step)) chosen = e1;
        else if (dirOk(r, e2, step)) chosen = e2;
        else {
          const Pose p = r.pose();
          chosen = bearingDeg(p.x_cm, p.y_cm, cfg::ARENA_W_CM * 0.5f, cfg::ARENA_H_CM * 0.5f);
        }
        prev_ = -32768;
      } else {
        const float hs[3] = {base_heading_, wrapDeg(base_heading_ + cfg::GRAD_SWEEP_DEG),
                             wrapDeg(base_heading_ - cfg::GRAD_SWEEP_DEG)};
        const int16_t vs[3] = {gC_, gL_, gR_};
        int16_t bestv = -32768;
        bool any = false;
        for (int i = 0; i < 3; ++i) {
          if (!dirOk(r, hs[i], step)) continue;
          if (!any || vs[i] > bestv) {
            bestv = vs[i];
            chosen = hs[i];
            any = true;
          }
        }
        if (!any) {
          const Pose p = r.pose();
          chosen = bearingDeg(p.x_cm, p.y_cm, cfg::ARENA_W_CM * 0.5f, cfg::ARENA_H_CM * 0.5f);
          prev_ = -32768;
        } else {
          prev_ = bestv;
        }
      }

      stepForward(r, chosen, step);
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

const char* GradientSearch::stateName() const {
  switch (ph_) {
    case Ph::SEEK_GOTO:
    case Ph::SEEK_SNIFF: return "SEEKING";
    case Ph::STEP: return "STEP";
    case Ph::STEP_SNIFF: return "SNIFF";
    case Ph::SWEEP_TURN_L:
    case Ph::SWEEP_SNIFF_L: return "SWEEP_L";
    case Ph::SWEEP_TURN_R:
    case Ph::SWEEP_SNIFF_R: return "SWEEP_R";
    case Ph::SWEEP_DECIDE: return "DECIDE";
    case Ph::RETURN: return "RETURN";
    case Ph::DONE: return "SOURCE_FOUND";
  }
  return "?";
}

}
