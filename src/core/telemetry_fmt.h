// ============================================================================
//  telemetry_fmt.h - dinh dang goi tin. Thuan chuoi ky tu, khong I/O.
//
//  Mot dong telemetry co hai cach doc:
//    * Dang MAY DOC (gui qua LoRa / Serial, cho tools/receiver.py):
//        $GS,t,algo,state,adc,norm,ppm,level,x,y,head,dist,cx,cy,best,fin*HH
//        HH = XOR checksum cua moi ky tu giua '$' va '*', in hexa 2 chu so.
//    * Dang NGUOI DOC (in ra man hinh tram, dung mau de cuong muc 11.4):
//        t=124.5s | ALGO=GRADIENT | ADC=612 | PPM=180 | LEVEL=HIGH |
//        STATE=SURGE | CELL=(3,5)
// ============================================================================
#pragma once
#include <cstddef>
#include <cstdint>

#include "irobot.h"

namespace gs {

struct TelemetrySample {
  uint32_t t_ms = 0;
  const char* algo = "?";
  const char* state = "?";
  uint16_t adc = 0;
  int16_t norm = 0;
  float ppm = 0.0f;
  AlarmLevel level = AlarmLevel::Safe;
  Pose pose;
  float dist_cm = 0.0f;
  int16_t best_norm = 0;
  bool finished = false;
};

// Tra ve so ky tu da ghi (khong ke '\0').
size_t buildCsv(char* out, size_t n, const TelemetrySample& s);

// Chuyen dong CSV thanh dang nguoi doc. Tra ve false neu dong khong hop le.
bool prettyFromCsv(const char* csv, char* out, size_t n);

// Kiem tra checksum cua mot dong '$...*HH'.
bool verifyChecksum(const char* line);

}  // namespace gs
