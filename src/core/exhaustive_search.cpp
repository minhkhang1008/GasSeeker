// ============================================================================
//  THUAT TOAN 1 - QUET TOAN BO (boustrophedon / exhaustive) - BASELINE
//
//  Quet zig-zag het luoi san, dung ngui tai tam moi o, ghi lai o co gia tri
//  cao nhat. Quet xong thi quay ve o do (neu bat EXH_RETURN_TO_BEST).
//
//  Dac diem: ti le thanh cong cao nhat nhung thoi gian dai nhat -> dung lam
//  moc so sanh cho H3.
//
//  Luu y: thuat toan nay KHONG dung dieu kien dung theo nong do. No luon quet
//  het san. Do la dinh nghia cua baseline - neu cho no dung som thi khong con
//  la "quet toan bo" nua va mat y nghia so sanh.
// ============================================================================
#include "search_algorithm.h"

namespace gs {

void ExhaustiveSearch::begin(IRobot& r) {
  path_.begin(1);  // stride 1 = quet tung o mot
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
  // Het diem quet.
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

  // --- an toan: va cham ---
  if (bump_.active()) {
    if (bump_.update(r)) {
      // Da ket luan roi thi KHONG duoc quay lai tim kiem nua. Neu dam vat can
      // tren duong ve diem cao nhat thi dung luon tai cho: ket luan la DIEM DO
      // cao nhat da ghi lai, ve duoc tan noi chi la co gang them.
      if (ph_ == Ph::RETURN) {
        r.cmdStop();
        ph_ = Ph::DONE;
      } else {
        nextWaypoint(r);  // bo qua o dang ket, di tiep
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

}  // namespace gs
