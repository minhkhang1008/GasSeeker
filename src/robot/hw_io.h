#pragma once
#include <Arduino.h>

#include "../core/irobot.h"

namespace hw {

enum class BtnEvent : uint8_t { NONE, SHORT_PRESS, LONG_PRESS };

void ioBegin();
void ioUpdate();

bool bumperLeft();
bool bumperRight();
bool bumperAny();

BtnEvent buttonPoll();

void statusColor(uint8_t r, uint8_t g, uint8_t b);
void statusByLevel(gs::AlarmLevel lv);
void statusOff();

void beep(uint16_t ms);
void beepPattern(uint8_t times);

}
