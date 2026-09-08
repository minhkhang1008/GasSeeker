#pragma once
#include <Arduino.h>

namespace hw {

void gasBegin();
uint16_t gasReadRaw();
float gasReadMv();

}
