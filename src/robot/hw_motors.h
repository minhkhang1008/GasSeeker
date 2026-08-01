// ============================================================================
//  hw_motors.h - dieu khien hai motor qua module TB6612FNG.
//  Quy uoc: pwm > 0 = tien, pwm < 0 = lui, don vi -255..255.
// ============================================================================
#pragma once
#include <Arduino.h>

namespace hw {

void motorsBegin();
// Dat toc do hai banh. Tu dong bu vung chet (deadband) cua motor.
void motorsSet(int left, int right);
void motorsBrake();   // ham ca hai chan -> phanh nhanh
void motorsCoast();   // tha troi
void motorsEnable(bool on);  // chan STBY cua TB6612
bool motorsEnabled();

}  // namespace hw
