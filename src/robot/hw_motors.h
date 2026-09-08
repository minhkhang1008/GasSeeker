#pragma once
#include <Arduino.h>

namespace hw {

void motorsBegin();
void motorsSet(int left, int right);
void motorsBrake();
void motorsCoast();
void motorsEnable(bool on);
bool motorsEnabled();

}
