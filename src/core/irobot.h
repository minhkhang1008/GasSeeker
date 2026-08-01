// ============================================================================
//  irobot.h - DUONG BIEN giua thuat toan va phan cung.
//
//  Day la file quan trong nhat cua kien truc: ba thuat toan tim nguon chi
//  duoc phep goi cac ham trong lop IRobot. Nho vay cung mot file .cpp thuat
//  toan chay duoc ca tren ESP32 (src/robot/) lan tren may tinh (src/sim/).
//
//  Quy uoc lenh chuyen dong: KHONG CHAN (non-blocking).
//    - Goi cmdForward()/cmdTurn() de dat mot lenh.
//    - Sau do goi motionBusy() moi vong lap; khi tra ve false la lenh xong.
//    - Dat lenh moi khi dang ban se HUY lenh cu.
// ============================================================================
#pragma once
#include <cstdint>

#include "geometry.h"

namespace gs {

// Muc canh bao cho nguoi giam sat (Lop 2 - chi de hien thi).
// Viet hoa chu dau chu khong viet hoa het: Arduino.h dinh nghia macro HIGH,
// se pha hong hang so mang ten HIGH. Ten gui di tren song van la "HIGH".
enum class AlarmLevel : uint8_t { Safe = 0, Detected = 1, High = 2, Critical = 3 };

const char* alarmLevelName(AlarmLevel lv);

struct GasReading {
  uint16_t raw = 0;         // Lop 1: ADC da qua trung binh truot
  int16_t normalized = 0;   // Lop 1: raw - baseline  (co the am)
  float ppm = 0.0f;         // Lop 2: nong do uoc luong, CHI de hien thi
  AlarmLevel level = AlarmLevel::Safe;  // Lop 2
  bool valid = false;       // false khi chua do xong baseline
};

class IRobot {
 public:
  virtual ~IRobot() {}

  // --- Thoi gian ---
  virtual uint32_t nowMs() const = 0;

  // --- Cam bien ---
  virtual GasReading gas() const = 0;
  virtual Pose pose() const = 0;
  virtual float travelledCm() const = 0;  // tong quang duong da di

  // --- Chuyen dong (khong chan) ---
  virtual void cmdForward(float cm) = 0;      // cm > 0 tien, cm < 0 lui
  virtual void cmdTurn(float delta_deg) = 0;  // tuong doi, + = quay trai (CCW)
  virtual void cmdStop() = 0;
  virtual bool motionBusy() const = 0;

  // --- An toan ---
  virtual bool bumped() const = 0;  // co cong tac va cham nao dang bi cham
  virtual void clearBump() = 0;

  // --- Ghi log su kien (Serial tren xe, stdout tren sim) ---
  virtual void log(const char* msg) = 0;
};

}  // namespace gs
