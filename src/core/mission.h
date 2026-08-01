// ============================================================================
//  mission.h - dieu phoi mot lan chay: chon thuat toan, dem gio, dem quang
//  duong, chan het gio. Dung chung cho firmware tren xe va cho simulator, de
//  hai ben do dac giong het nhau.
// ============================================================================
#pragma once
#include <cstdint>

#include "search_algorithm.h"
#include "telemetry_fmt.h"

namespace gs {

enum class MissionResult : uint8_t { RUNNING, FOUND, TIMEOUT, ABORTED };

class Mission {
 public:
  void begin(IRobot& r, Algo a);
  void update(IRobot& r);
  void abort(IRobot& r);

  MissionResult result() const { return result_; }
  bool running() const { return result_ == MissionResult::RUNNING; }
  Algo algo() const { return algo_; }
  const char* stateName() const;
  BestPoint best() const;

  uint32_t elapsedMs(const IRobot& r) const { return r.nowMs() - t0_; }
  float pathCm(const IRobot& r) const { return r.travelledCm() - start_dist_cm_; }

  TelemetrySample sample(const IRobot& r) const;

 private:
  SearchAlgorithm* alg_ = nullptr;
  Algo algo_ = Algo::EXHAUSTIVE;
  MissionResult result_ = MissionResult::ABORTED;
  uint32_t t0_ = 0;
  float start_dist_cm_ = 0.0f;
};

}  // namespace gs
