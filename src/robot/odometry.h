// ============================================================================
//  odometry.h - dinh vi bang dead reckoning (de cuong muc 11.3).
//
//  Quang duong : encoder quang mot kenh. Cau hinh hien tai chi co MOT encoder
//                (banh sau trai) -> cfg::ENCODER_COUNT = 1.
//                Encoder mot kenh KHONG biet chieu quay -> chieu duoc suy ra
//                tu lenh dang cap cho motor (odomSetWheelDir).
//  Huong       : tich phan gyro Z cua MPU6050 (chinh xac hon nhieu so voi
//                lay hieu so xung hai banh, vi banh hay truot khi quay).
//                Neu khong co MPU6050 -> tu dong quay ve dung hieu so encoder.
//
//  Sai so tich luy theo thoi gian. Day la han che da nêu o muc 8.4 de cuong.
// ============================================================================
#pragma once
#include <Arduino.h>

#include "../core/geometry.h"

namespace hw {

void odomBegin();
void odomReset(float x_cm, float y_cm, float heading_deg);
// Chieu quay dang duoc cap cho tung banh: -1 lui, 0 dung, +1 tien.
void odomSetWheelDir(int left_dir, int right_dir);
void odomUpdate();  // goi deu dan, khoang 100 Hz

gs::Pose odomPose();
float odomTravelledCm();
bool odomUsingGyro();

// "Doan duong" phuc vu mot lenh chuyen dong: dat lai truoc moi lenh.
void odomSegmentReset();
float odomSegmentCm();       // quang duong trung binh hai banh, luon >= 0
float odomSegmentTurnDeg();  // goc da quay (co dau)

long odomTicksL();
long odomTicksR();
bool odomHasRightEncoder();

}  // namespace hw
