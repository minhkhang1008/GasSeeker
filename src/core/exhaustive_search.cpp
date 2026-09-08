#include "search_algorithm.h"

namespace gs {

void ExhaustiveSearch::begin(IRobot& r) {
  path_.begin(1);
  best_.reset();
  bump_.reset();
  nav_.abort(r);
  ph_ = Ph::GOTO;
  r.log("EXHAUSTIVE: bat dau quet toan bo luoi");
  nextWaypoint(r);
}

void ExhaustiveSearch::nextWaypoint(IRobot& r) {
  float x, y;
  if (path_.next(x, y)) {
    nav_.goTo(r, x, y);
    ph_ = Ph::GOTO;
    return;
  }
  const BestPoint& b = best_.get();
  if (cfg::RETURN_TO_BEST && b.valid) {
    r.log("EXHAUSTIVE: quet xong, quay ve diem cao nhat");
    nav_.goTo(r, b.x_cm, b.y_cm);
    ph_ = Ph::RETURN;
  } else {
    r.cmdStop();
    ph_ = Ph::DONE;
  }
}

void ExhaustiveSearch::update(IRobot& r) {
  if (ph_ == Ph::DONE) return;

  if (bump_.active()) {
    if (bump_.update(r)) {
      if (ph_ == Ph::RETURN) {
        r.cmdStop();
        ph_ = Ph::DONE;
      } else {
        nextWaypoint(r);
      }
    }
    return;
  }
  if (bump_.triggerIfBumped(r)) {
    nav_.abort(r);
    return;
  }

  switch (ph_) {
    case Ph::GOTO: {
      const bool arrived = !nav_.busy() || nav_.update(r);
      if (arrived) {
        r.cmdStop();
        sniff_.start(r.nowMs());
        ph_ = Ph::SNIFF;
      }
      break;
    }

    case Ph::SNIFF: {
      if (sniff_.update(r)) {
        best_.feed(r, sniff_.value(), sniff_.rawValue(), sniff_.ppm());
        nextWaypoint(r);
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

const char* ExhaustiveSearch::stateName() const {
  switch (ph_) {
    case Ph::GOTO: return "SCAN";
    case Ph::SNIFF: return "SNIFF";
    case Ph::RETURN: return "RETURN";
    case Ph::DONE: return "SOURCE_FOUND";
  }
  return "?";
}

}
