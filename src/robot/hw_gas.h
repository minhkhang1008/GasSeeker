// ============================================================================
//  hw_gas.h - doc ADC tu module MQ-3 (chan AO qua mach chia ap).
//
//  Tra ve HAI gia tri, dung voi hai lop du lieu cua de cuong muc 11.1:
//    raw : so dem ADC 12 bit  -> Lop 1, thuat toan dung
//    mv  : dien ap thuc (mV) do bang bang hieu chuan trong eFuse cua ESP32
//          -> Lop 2, de tinh Rs roi quy ra ppm
// ============================================================================
#pragma once
#include <Arduino.h>

namespace hw {

void gasBegin();
uint16_t gasReadRaw();
float gasReadMv();

}  // namespace hw
