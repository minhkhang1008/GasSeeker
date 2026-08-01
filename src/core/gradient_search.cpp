// ============================================================================
//  THUAT TOAN 2 - BAM GRADIENT (chemotaxis)
//
//  De cuong muc 13.2 mo ta hai bien the. Bien the duoc cai dat o day ket hop
//  ca hai, vi mot ly do VAT LY cu the:
//
//     Cam bien MQ-3 hoi phuc rat cham (datasheet: recovery time <= 30 s).
//     Neu quay tai cho va do lien tiep ba huong, ba phep do cach nhau chi vai
//     giay -> gia tri doc duoc con mang "ky uc" cua huong do truoc do, khien
//     ba so lieu bi lech he thong theo THU TU DO chu khong theo khong gian.
//
//  Vi vay:
//     - Khi nong do CON TANG: cu di thang theo huong dang di. Hai phep do cach
//       nhau GRAD_STEP_CM va vai giay -> chenh lech do khong gian at hon do tre.
//     - Chi khi nong do GIAM moi bo cong quet ba huong de chon huong moi.
//
//  Vong lap:
//      tien mot buoc -> do
//         con tang  -> tien tiep theo huong cu
//         da giam   -> quet trai / quet phai -> chon huong tot nhat -> tien
//
//  Ket thuc: do tre cua cam bien khien robot thuong VUOT QUA dinh roi moi biet.
//  Do do khi thoa dieu kien dung, robot QUAY LAI diem do duoc gia tri cao nhat
//  roi moi bao "da tim thay nguon".
//
//  Diem yeu co y de lo (phuc vu H2): neu tin hieu dut quang, gradient khong co
//  co che tim lai - no chi di lang thang. Do la dieu surge-casting khac phuc.
// ============================================================================
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

// Ghi nhan mot phep do: luu diem cao nhat VA cap nhat bo chong ket.
// Hai viec nay doc lap nhau - xem chu thich cua StallGuard.
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

  // --- an toan: va cham ---
  if (bump_.active()) {
    if (bump_.update(r)) {
      // Da ket luan roi thi KHONG duoc quay lai tim kiem nua. Neu dam vat can
      // tren duong ve diem cao nhat thi dung luon tai cho: ket luan la DIEM DO
      // cao nhat da ghi lai, ve duoc tan noi chi la co gang them.
      if (ph_ == Ph::RETURN) {
        r.cmdStop();
        ph_ = Ph::DONE;
      } else if (!acquired_) {
        seekNext(r);
      } else {
        startSweep(r);  // dam vat can -> tim huong khac
      }
    }
    return;
  }
  if (bump_.triggerIfBumped(r)) {
    nav_.abort(r);
    return;
  }

  switch (ph_) {
    // ---------------- SEEK: chua bat duoc luong khi ----------------
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
        // Bat dau bang mot vong quet de chon huong di dau tien.
        startSweep(r);
      } else {
        seekNext(r);
      }
      break;
    }

    // ---------------- di thang khi con tang ----------------
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
          stepForward(r, h, cfg::GRAD_STEP_CM);  // con tang -> giu nguyen huong
          break;
        }
        // Ra khoi san -> buoc phai quet lai.
      }
      ++no_gain_steps_;
      startSweep(r);
      break;
    }

    // ---------------- nong do giam -> quet ba huong ----------------
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
      gC_ = prev_;  // gia tri do duoc o huong giua, do o buoc truoc
      ph_ = Ph::SWEEP_DECIDE;
      break;
    }

    case Ph::SWEEP_DECIDE: {
      float step = cfg::GRAD_STEP_CM;
      float chosen = base_heading_;

      if (no_gain_steps_ >= cfg::GRAD_STUCK_STEPS) {
        // Ket o cuc tri dia phuong / vung nhieu -> quay goc lon roi di xa hon.
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
        prev_ = -32768;  // moc so sanh cu khong con y nghia
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

}  // namespace gs
