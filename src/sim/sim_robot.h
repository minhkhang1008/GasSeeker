// ============================================================================
//  sim_robot.h - hien thuc IRobot bang mo phong tren may tinh.
//
//  Giu HAI vi tri song song:
//    true_ : vi tri THAT (dung de cham diem sai so, va de lay nong do khi)
//    odom_ : vi tri robot TIN (dead reckoning) - day la thu thuat toan nhin thay
//  Do lech giua hai cai la sai so dead-reckoning, dung nhu tren xe that.
// ============================================================================
#pragma once
#include <cstdint>
#include <random>

#include "../core/gas.h"
#include "../core/irobot.h"
#include "plume.h"

namespace sim {

class SimRobot : public gs::IRobot {
 public:
  void begin(Env env, float src_x, float src_y, uint32_t seed);
  // Chay khong tai de bo loc + baseline on dinh truoc khi vao nhiem vu.
  void warmup(float seconds);
  void step(float dt_s);

  void setVerbose(bool v) { verbose_ = v; }

  // --- IRobot ---
  uint32_t nowMs() const override { return t_ms_; }
  gs::GasReading gas() const override { return gasproc_.reading(); }
  gs::Pose pose() const override { return odom_; }
  float travelledCm() const override { return travelled_cm_; }
  void cmdForward(float cm) override;
  void cmdTurn(float delta_deg) override;
  void cmdStop() override;
  bool motionBusy() const override { return mode_ != Mode::IDLE; }
  bool bumped() const override { return bumped_; }
  void clearBump() override { bumped_ = false; }
  void log(const char* msg) override;

  // --- cho phan cham diem (thuat toan KHONG duoc dung) ---
  gs::Pose truePose() const { return true_; }
  float srcX() const { return plume_.srcX(); }
  float srcY() const { return plume_.srcY(); }
  uint16_t lastAdc() const { return last_adc_; }

 private:
  enum class Mode : uint8_t { IDLE, DRIVE, TURN, BRAKE };

  void sampleGas(float dt_s);

  Mode mode_ = Mode::IDLE;
  float remain_cm_ = 0.0f;
  float remain_deg_ = 0.0f;
  float dir_ = 1.0f;
  uint32_t brake_until_ms_ = 0;

  gs::Pose true_;
  gs::Pose odom_;
  float travelled_cm_ = 0.0f;
  bool bumped_ = false;

  uint32_t t_ms_ = 0;
  double t_acc_s_ = 0.0;
  uint32_t next_gas_ms_ = 0;
  uint16_t last_adc_ = 0;

  Plume plume_;
  Mq3Model mq3_;
  gs::GasProcessor gasproc_;

  // sai so dead-reckoning cua lan chay nay
  float dist_scale_err_ = 0.0f;
  float heading_drift_dps_ = 0.0f;

  std::mt19937 rng_;
  std::normal_distribution<float> gauss_{0.0f, 1.0f};
  bool verbose_ = false;
};

}  // namespace sim
