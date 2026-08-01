#include "hw_imu.h"

#include <Wire.h>

#include "../core/config.h"

namespace hw {

static constexpr uint8_t ADDR = 0x68;
static constexpr uint8_t REG_PWR_MGMT_1 = 0x6B;
static constexpr uint8_t REG_GYRO_CONFIG = 0x1B;
static constexpr uint8_t REG_CONFIG = 0x1A;
static constexpr uint8_t REG_GYRO_ZOUT_H = 0x47;
static constexpr uint8_t REG_WHO_AM_I = 0x75;

// GYRO_CONFIG = 0x08 -> dai do +/-500 deg/s -> 65.5 LSB tren moi deg/s.
static constexpr float LSB_PER_DPS = 65.5f;

static bool ok_ = false;
static float bias_dps_ = 0.0f;

static bool writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

static bool readRegs(uint8_t reg, uint8_t* buf, uint8_t n) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)ADDR, (int)n) != n) return false;
  for (uint8_t i = 0; i < n; ++i) buf[i] = Wire.read();
  return true;
}

bool imuBegin() {
  Wire.begin(cfg::pin::I2C_SDA, cfg::pin::I2C_SCL, 400000);
  delay(50);

  uint8_t who = 0;
  if (!readRegs(REG_WHO_AM_I, &who, 1)) {
    ok_ = false;
    return false;
  }
  // MPU6050 tra ve 0x68; mot so ban sao tra ve 0x70/0x72/0x98 - van chap nhan.
  if (who == 0x00 || who == 0xFF) {
    ok_ = false;
    return false;
  }

  ok_ = writeReg(REG_PWR_MGMT_1, 0x01);   // thoat sleep, clock = PLL truc X
  delay(20);
  ok_ &= writeReg(REG_CONFIG, 0x03);      // loc thong thap 44 Hz -> bot rung motor
  ok_ &= writeReg(REG_GYRO_CONFIG, 0x08); // +/-500 deg/s
  delay(20);
  return ok_;
}

bool imuOk() { return ok_; }
float imuBias() { return bias_dps_; }

static float rawGyroZ() {
  uint8_t b[2];
  if (!readRegs(REG_GYRO_ZOUT_H, b, 2)) return 0.0f;
  const int16_t raw = (int16_t)((b[0] << 8) | b[1]);
  return (float)raw / LSB_PER_DPS;
}

void imuCalibrateBias(uint32_t duration_ms) {
  if (!ok_) {
    bias_dps_ = 0.0f;
    return;
  }
  double acc = 0.0;
  uint32_t n = 0;
  const uint32_t t0 = millis();
  while (millis() - t0 < duration_ms) {
    acc += rawGyroZ();
    ++n;
    delay(5);
  }
  bias_dps_ = (n > 0) ? (float)(acc / (double)n) : 0.0f;
}

float imuGyroZ() {
  if (!ok_) return 0.0f;
  return rawGyroZ() - bias_dps_;
}

}  // namespace hw
