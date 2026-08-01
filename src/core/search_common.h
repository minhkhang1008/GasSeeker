// ============================================================================
//  search_common.h - cac khoi dung chung cho ca ba thuat toan tim nguon.
// ============================================================================
#pragma once
#include <cstdint>

#include "config.h"
#include "gas.h"
#include "geometry.h"
#include "irobot.h"

namespace gs {

// ---------------------------------------------------------------------------
//  Diem do tot nhat tu truoc den nay.
// ---------------------------------------------------------------------------
struct BestPoint {
  float x_cm = 0.0f;
  float y_cm = 0.0f;
  int16_t normalized = -32768;
  uint16_t raw = 0;
  float ppm = 0.0f;
  uint32_t t_ms = 0;
  bool valid = false;
};

class BestTracker {
 public:
  void reset() { b_ = BestPoint{}; }
  // Tra ve true neu day la diem tot nhat moi.
  bool feed(const IRobot& r, int16_t normalized, uint16_t raw, float ppm);
  const BestPoint& get() const { return b_; }

 private:
  BestPoint b_;
};

// ---------------------------------------------------------------------------
//  Navigator - gom hai buoc "quay roi di" thanh mot lenh khong chan.
// ---------------------------------------------------------------------------
class Navigator {
 public:
  void goTo(IRobot& r, float x_cm, float y_cm);
  void turnTo(IRobot& r, float heading_deg);
  void turnBy(IRobot& r, float delta_deg);
  void forward(IRobot& r, float cm);
  void abort(IRobot& r);

  // Goi moi vong lap. Tra ve true DUNG MOT LAN khi lenh hoan tat.
  bool update(IRobot& r);
  bool busy() const { return ph_ != Ph::IDLE; }

 private:
  enum class Ph : uint8_t { IDLE, TURN, DRIVE };
  Ph ph_ = Ph::IDLE;
  float pending_drive_cm_ = 0.0f;
};

// ---------------------------------------------------------------------------
//  StopDetector - dieu kien dung cua de cuong muc 13.2, phai thoa DONG THOI:
//    (a) gia tri khi vuot nguong cao
//    (b) khong con tang dang ke
//    (c) duy tri trong mot khoang thoi gian
//
//  Luu y ve thoi gian: phai co HAI phep do moi ket luan duoc "khong con tang",
//  nen dong ho cua dieu kien (c) chi bat dau tu phep do THU HAI. Thoi gian toi
//  thieu tu luc toi vung dac den luc dung = mot chu ky do + STOP_HOLD_MS.
// ---------------------------------------------------------------------------
class StopDetector {
 public:
  void reset();
  // Goi sau MOI phep do (sniff). Tra ve true khi da du dieu kien dung.
  bool feed(int16_t normalized, uint32_t now_ms);
  int16_t best() const { return best_; }
  bool improvedLast() const { return improved_; }

 private:
  int16_t best_ = -32768;
  bool holding_ = false;
  bool improved_ = false;
  uint32_t hold_start_ = 0;
};

// ---------------------------------------------------------------------------
//  StallGuard - chong ket: "da qua lau khong tien trien" thi phai ket luan.
//
//  Diem tinh te: mot ky luc moi CHUA CHAC la tien trien. Trong truong nhieu,
//  gia tri cao hon ky luc cu dung 1 dem ADC xuat hien lien tuc; neu lay do lam
//  moc reset thi bo dem khong bao gio day va robot chay toi het gio (loi that
//  da bi bo test bat duoc). Vi vay chi coi la tien trien khi vuot moc cu qua
//  PLATEAU_EPS - cung nguong "con tang" dung o moi noi khac.
// ---------------------------------------------------------------------------
class StallGuard {
 public:
  void reset() {
    ref_ = -32768;
    n_ = 0;
  }
  // Goi sau MOI phep do. Tra ve true khi nen dung lai va ket luan.
  bool feed(int16_t normalized);
  int count() const { return n_; }

 private:
  int16_t ref_ = -32768;
  int n_ = 0;
};

// ---------------------------------------------------------------------------
//  BumpRecovery - phan ung khi cong tac va cham bi cham: lui lai roi quay.
// ---------------------------------------------------------------------------
class BumpRecovery {
 public:
  void reset() { ph_ = Ph::IDLE; }
  // Neu dang co va cham -> bat dau quy trinh lui/quay, tra ve true.
  bool triggerIfBumped(IRobot& r);
  bool active() const { return ph_ != Ph::IDLE; }
  // Tra ve true DUNG MOT LAN khi da xu ly xong.
  bool update(IRobot& r);

 private:
  enum class Ph : uint8_t { IDLE, BACKING, TURNING };
  Ph ph_ = Ph::IDLE;
  int side_ = 1;
};

// ---------------------------------------------------------------------------
//  LawnmowerPath - sinh chuoi diem quet zig-zag (boustrophedon) tren luoi san.
//  stride = 1 -> quet toan bo (thuat toan 1).
//  stride = SEEK_STRIDE -> quet tho de "bat luong" (giai doan SEEK).
// ---------------------------------------------------------------------------
class LawnmowerPath {
 public:
  void begin(int stride);
  bool next(float& x_cm, float& y_cm);  // false khi da het diem
  bool done() const { return done_; }
  int index() const { return k_; }
  int total() const { return total_; }

 private:
  int stride_ = 1;
  int k_ = 0;
  int nx_ = 0, ny_ = 0;
  int total_ = 0;
  bool done_ = false;
};

}  // namespace gs
