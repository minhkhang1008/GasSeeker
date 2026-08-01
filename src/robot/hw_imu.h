// ============================================================================
//  hw_imu.h - MPU6050 (GY-521) doc truc tiep qua thanh ghi, khong dung thu vien.
//
//  Robot chi can DUY NHAT mot dai luong: toc do goc quanh truc Z (gyro Z), de
//  tich phan ra huong. Gia toc ke khong giup gi cho goc yaw nen bo qua.
//  Viet truc tiep ~80 dong de khong phu thuoc thu vien ngoai.
// ============================================================================
#pragma once
#include <Arduino.h>

namespace hw {

bool imuBegin();
bool imuOk();
// Robot PHAI dung yen trong suot qua trinh nay.
void imuCalibrateBias(uint32_t duration_ms);
// Toc do goc quanh truc Z, deg/s, da tru bias. + = quay trai (CCW).
float imuGyroZ();
float imuBias();

}  // namespace hw
