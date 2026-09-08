#pragma once
#include <cstdint>

#include "search_common.h"

namespace gs {

enum class Algo : uint8_t {
  EXHAUSTIVE = 0,
  GRADIENT = 1,
  SURGE_CAST = 2,
  COUNT = 3
};

const char* algoName(Algo a);
const char* algoShortName(Algo a);

class SearchAlgorithm {
 public:
  virtual ~SearchAlgorithm() {}
  virtual void begin(IRobot& r) = 0;
  virtual void update(IRobot& r) = 0;
  virtual bool finished() const = 0;
  virtual const char* stateName() const = 0;
  virtual Algo algo() const = 0;
  virtual BestPoint best() const = 0;
};

class ExhaustiveSearch : public SearchAlgorithm {
 public:
  void begin(IRobot& r) override;
  void update(IRobot& r) override;
  bool finished() const override { return ph_ == Ph::DONE; }
  const char* stateName() const override;
  Algo algo() const override { return Algo::EXHAUSTIVE; }
  BestPoint best() const override { return best_.get(); }

 private:
  void nextWaypoint(IRobot& r);

  enum class Ph : uint8_t { GOTO, SNIFF, RETURN, DONE };
  Ph ph_ = Ph::GOTO;
  LawnmowerPath path_;
  Navigator nav_;
  Sniffer sniff_;
  BestTracker best_;
  BumpRecovery bump_;
};

class GradientSearch : public SearchAlgorithm {
 public:
  void begin(IRobot& r) override;
  void update(IRobot& r) override;
  bool finished() const override { return ph_ == Ph::DONE; }
  const char* stateName() const override;
  Algo algo() const override { return Algo::GRADIENT; }
  BestPoint best() const override { return best_.get(); }

 private:
  void seekNext(IRobot& r);
  void stepForward(IRobot& r, float heading_deg, float step_cm);
  void startSweep(IRobot& r);
  void finish(IRobot& r);
  bool dirOk(const IRobot& r, float heading_deg, float step_cm) const;

  enum class Ph : uint8_t {
    SEEK_GOTO,
    SEEK_SNIFF,
    STEP,
    STEP_SNIFF,
    SWEEP_TURN_L, SWEEP_SNIFF_L,
    SWEEP_TURN_R, SWEEP_SNIFF_R,
    SWEEP_DECIDE,
    RETURN,
    DONE
  };
  Ph ph_ = Ph::SEEK_GOTO;

  Navigator nav_;
  Sniffer sniff_;
  BestTracker best_;
  BumpRecovery bump_;
  StopDetector stop_;
  LawnmowerPath seek_;

  bool acquired_ = false;
  float base_heading_ = 0;
  int16_t gC_ = 0, gL_ = 0, gR_ = 0;
  int16_t prev_ = -32768;
  int no_gain_steps_ = 0;
  StallGuard stall_;
};

class SurgeCastSearch : public SearchAlgorithm {
 public:
  void begin(IRobot& r) override;
  void update(IRobot& r) override;
  bool finished() const override { return ph_ == Ph::DONE; }
  const char* stateName() const override;
  Algo algo() const override { return Algo::SURGE_CAST; }
  BestPoint best() const override { return best_.get(); }

 private:
  void seekNext(IRobot& r);
  void startSurge(IRobot& r);
  void startCast(IRobot& r);
  void finish(IRobot& r);
  bool dirOk(const IRobot& r, float heading_deg, float step_cm) const;

  enum class Ph : uint8_t {
    SEEK_GOTO, SEEK_SNIFF,
    SURGE_MOVE, SURGE_SNIFF,
    CAST_MOVE, CAST_SNIFF,
    RETURN,
    DONE
  };
  Ph ph_ = Ph::SEEK_GOTO;

  Navigator nav_;
  Sniffer sniff_;
  BestTracker best_;
  BumpRecovery bump_;
  StopDetector stop_;
  LawnmowerPath seek_;

  uint32_t last_detect_ms_ = 0;
  float cast_amp_ = cfg::SC_CAST_STEP_CM;
  int cast_side_ = +1;
  int cast_legs_ = 0;
  StallGuard stall_;
};

SearchAlgorithm* makeAlgorithm(Algo a);

}
