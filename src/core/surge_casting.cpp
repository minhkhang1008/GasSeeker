// ============================================================================
//  THUAT TOAN 3 - SURGE-CASTING (mo phong hanh vi con trung tim mui)
//
//  May trang thai (de cuong muc 13.3):
//      SEEKING -> SURGE -> (mat tin hieu) -> CAST_LEFT/CAST_RIGHT -> SURGE
//                                                                 -> SOURCE_FOUND
//
//  SURGE : con thay khi thi tien THANG NGUOC HUONG GIO.
//  CAST  : mat khi thi quet ngang VUONG GOC voi huong gio, doi ben lien tuc
//          voi bien do tang dan cho toi khi bat lai duoc luong.
//
//  Huong gio KHONG do bang cam bien. No la hang so cfg::WIND_FROM_DEG khai bao
//  truoc theo cach bo tri quat trong san (de cuong muc 11.2). Bao cao PHAI ghi
//  ro dieu nay.
//
//  Y tuong cot loi khien H2 dung: khi tin hieu dut quang, "mat tin hieu" khong
//  con la that bai ma tro thanh mot hanh vi tim kiem CO DINH HUONG.
// ============================================================================
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

// Do tre cua MQ-3, robot thuong vuot qua dinh roi moi nhan ra -> quay lai
// diem do duoc gia tri cao nhat truoc khi ket luan.
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

// Tien mot buoc nguoc huong gio.
void SurgeCastSearch::startSurge(IRobot& r) {
  const float upwind = cfg::WIND_FROM_DEG;
  if (!dirOk(r, upwind, cfg::SC_SURGE_STEP_CM)) {
    // Da toi mep san phia dau gio: khong tien them duoc nua. Nguon chac chan
    // nam lech sang mot ben -> phai cast voi bien do TANG DAN. Neu de
    // cast_legs_ = 0 o day, bien do se khong bao gio lon len va robot ket
    // cung o goc san (loi da gap khi chay mo phong).
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

// Quet ngang vuong goc huong gio, doi ben va tang bien do sau moi lan.
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
    // Ben nay dung tuong -> thu ben doi dien.
    cast_side_ = -cast_side_;
    heading = wrapDeg(cfg::WIND_FROM_DEG + cast_side_ * 90.0f);
    if (!dirOk(r, heading, cast_amp_)) {
      // Ca hai ben deu khong di duoc -> bien do lon qua, quay ve SEEKING.
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

  // --- an toan: va cham ---
  if (bump_.active()) {
    if (bump_.update(r)) {
      // Da ket luan roi thi KHONG duoc quay lai tim kiem nua. Neu dam vat can
      // tren duong ve diem cao nhat thi dung luon tai cho: ket luan la DIEM DO
      // cao nhat da ghi lai, ve duoc tan noi chi la co gang them.
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
    // ---------------- SEARCHING ----------------
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

    // ---------------- SURGE ----------------
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
        startSurge(r);  // moi mat mot chut, van con quan tinh tien len
      }
      break;
    }

    // ---------------- CAST ----------------
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

    // ---------------- quay ve diem cao nhat ----------------
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

}  // namespace gs
