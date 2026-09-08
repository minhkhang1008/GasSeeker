#pragma once
#include <Arduino.h>

namespace hw {

void motionBegin();
void motionForward(float cm);
void motionTurn(float delta_deg);
void motionStop();
bool motionBusy();
void motionUpdate();
const char* motionStateName();

}
