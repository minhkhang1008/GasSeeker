#pragma once
#include <Arduino.h>

namespace hw {

bool imuBegin();
bool imuOk();
void imuCalibrateBias(uint32_t duration_ms);
float imuGyroZ();
float imuBias();

}
