#include "mission.h"

namespace gs {

void Mission::begin(IRobot& r, Algo a) {
  algo_ = a;
  alg_ = makeAlgorithm(a);
  t0_ = r.nowMs();
  start_dist_cm_ = r.travelledCm();
  result_ = MissionResult::RUNNING;
  r.clearBump();
  alg_->begin(r);
}

void Mission::update(IRobot& r) {
  if (result_ != MissionResult::RUNNING || !alg_) return;

  if (elapsedMs(r) >= cfg::MISSION_TIMEOUT_MS) {
    r.log("MISSION: het thoi gian cho phep -> dung");
    r.cmdStop();
    result_ = MissionResult::TIMEOUT;
    return;
  }

  alg_->update(r);

  if (alg_->finished()) {
    r.cmdStop();
    result_ = MissionResult::FOUND;
  }
}

void Mission::abort(IRobot& r) {
  r.cmdStop();
  if (result_ == MissionResult::RUNNING) result_ = MissionResult::ABORTED;
}

const char* Mission::stateName() const {
  if (!alg_) return "IDLE";
  switch (result_) {
    case MissionResult::TIMEOUT: return "TIMEOUT";
    case MissionResult::ABORTED: return "ABORTED";
    default: return alg_->stateName();
  }
}

BestPoint Mission::best() const {
  return alg_ ? alg_->best() : BestPoint{};
}

TelemetrySample Mission::sample(const IRobot& r) const {
  const GasReading g = r.gas();
  TelemetrySample s;
  s.t_ms = elapsedMs(r);
  s.algo = algoShortName(algo_);
  s.state = stateName();
  s.adc = g.raw;
  s.norm = g.normalized;
  s.ppm = g.ppm;
  s.level = g.level;
  s.pose = r.pose();
  s.dist_cm = pathCm(r);
  s.best_norm = alg_ ? alg_->best().normalized : 0;
  s.finished = (result_ != MissionResult::RUNNING);
  return s;
}

}  // namespace gs
