// ============================================================================
//  hw_io.h - cong tac va cham, nut bam, LED trang thai, coi.
//
//  Tan dung phan cung co san tren ESP32-S3 DevKitC-1 de khong ton chan GPIO:
//    - Nut BOOT (GPIO0) lam nut dieu khien: nhan ngan = Start/Stop,
//      nhan giu = doi thuat toan.
//    - LED RGB dia chi (GPIO48) lam den bao muc canh bao, dung dung bang mau
//      trong de cuong muc 11.1c: xanh la / vang / cam / do.
// ============================================================================
#pragma once
#include <Arduino.h>

#include "../core/irobot.h"

namespace hw {

enum class BtnEvent : uint8_t { NONE, SHORT_PRESS, LONG_PRESS };

void ioBegin();
void ioUpdate();  // goi moi vong lap: chong doi nut, tat coi dung gio

bool bumperLeft();
bool bumperRight();
bool bumperAny();

BtnEvent buttonPoll();  // tra ve su kien MOT LAN roi tu xoa

void statusColor(uint8_t r, uint8_t g, uint8_t b);
void statusByLevel(gs::AlarmLevel lv);
void statusOff();

void beep(uint16_t ms);
void beepPattern(uint8_t times);  // n tieng ngan (khong chan vong lap)

}  // namespace hw
