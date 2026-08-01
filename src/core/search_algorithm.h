// ============================================================================
//  search_algorithm.h - giao dien chung cua ba chien luoc do tim nguon.
//
//  Ca ba deu tuan theo cung mot khuon:
//      begin(r)          - khoi tao truoc khi chay
//      update(r)         - goi lien tuc trong vong lap chinh, KHONG duoc chan
//      finished()        - da ket luan xong chua
//      stateName()       - ten trang thai hien tai (de gui ve telemetry)
//      best()            - diem do co nong do cao nhat -> ket luan vi tri nguon
// ============================================================================
#pragma once
#include <cstdint>

#include "search_common.h"

namespace gs {

enum class Algo : uint8_t {
  EXHAUSTIVE = 0,  // quet toan bo  (baseline)
  GRADIENT = 1,    // bam gradient  (quet 3 huong)
  SURGE_CAST = 2,  // surge-casting (mo phong hanh vi con trung)
  COUNT = 3
};

const char* algoName(Algo a);       // "EXHAUSTIVE" / "GRADIENT" / "SURGE_CAST"
const char* algoShortName(Algo a);  // "EXH" / "GRA" / "SUR"

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

// ---------------------------------------------------------------------------
//  1. Quet toan bo - de cuong muc 13.1
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
//  2. Bam gradient - de cuong muc 13.2 (ban "quet 3 huong")
// ---------------------------------------------------------------------------
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
  void finish(IRobot& r);  // quay ve diem cao nhat roi ket thuc
  // Buoc tien theo huong nay co ra ngoai san khong?
  bool dirOk(const IRobot& r, float heading_deg, float step_cm) const;

  enum class Ph : uint8_t {
    SEEK_GOTO,     // chua thay khi -> quet tho de bat luong
    SEEK_SNIFF,
    STEP,          // dang tien mot buoc theo huong hien tai
    STEP_SNIFF,    // do sau khi tien -> con tang thi di tiep
    SWEEP_TURN_L, SWEEP_SNIFF_L,   // nong do giam -> thu ba huong
    SWEEP_TURN_R, SWEEP_SNIFF_R,
    SWEEP_DECIDE,
    RETURN,        // quay ve diem do cao nhat
    DONE
  };
  Ph ph_ = Ph::SEEK_GOTO;

  Navigator nav_;
  Sniffer sniff_;
  BestTracker best_;
  BumpRecovery bump_;
  StopDetector stop_;
  LawnmowerPath seek_;

  bool acquired_ = false;   // da tung phat hien khi chua
  float base_heading_ = 0;  // huong luc bat dau quet 3 huong
  int16_t gC_ = 0, gL_ = 0, gR_ = 0;
  int16_t prev_ = -32768;   // gia tri o buoc truoc, de biet dang tang hay giam
  int no_gain_steps_ = 0;
  StallGuard stall_;        // chong ket khi khong con tien trien
};

// ---------------------------------------------------------------------------
//  3. Surge-casting - de cuong muc 13.3
// ---------------------------------------------------------------------------
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
  void finish(IRobot& r);  // quay ve diem cao nhat roi ket thuc
  bool dirOk(const IRobot& r, float heading_deg, float step_cm) const;

  enum class Ph : uint8_t {
    SEEK_GOTO, SEEK_SNIFF,   // SEARCHING
    SURGE_MOVE, SURGE_SNIFF, // SURGE
    CAST_MOVE, CAST_SNIFF,   // CAST_LEFT / CAST_RIGHT
    RETURN,                  // quay ve diem do cao nhat
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
  int cast_side_ = +1;  // +1 = trai so voi huong nguoc gio
  int cast_legs_ = 0;
  StallGuard stall_;    // chong ket khi khong con tien trien
};

// ---------------------------------------------------------------------------
//  Nha may: tra ve doi tuong tinh (khong cap phat dong -> an toan tren MCU).
// ---------------------------------------------------------------------------
SearchAlgorithm* makeAlgorithm(Algo a);

}  // namespace gs
