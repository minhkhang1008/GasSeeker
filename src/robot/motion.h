// ============================================================================
//  motion.h - hai nguyen thuy chuyen dong vong kin, KHONG CHAN vong lap chinh.
//
//    motionForward(cm) : di thang mot doan, giu huong bang gyro
//    motionTurn(deg)   : quay tai cho mot goc tuong doi (+ = trai / CCW)
//
//  Sau moi lenh co mot pha PHANH ngan de xe dung han truoc khi do khi -
//  neu do trong luc xe con truot, gia tri se khong ung voi vi tri nao ca.
// ============================================================================
#pragma once
#include <Arduino.h>

namespace hw {

void motionBegin();
void motionForward(float cm);
void motionTurn(float delta_deg);
void motionStop();
bool motionBusy();
void motionUpdate();  // goi deu dan, khoang 50 Hz
const char* motionStateName();

}  // namespace hw
