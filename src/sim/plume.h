#pragma once
#include <cstdint>
#include <random>
#include <vector>

#include "sim_config.h"

namespace sim {

enum class Env : uint8_t { DIFFUSION = 0, INTERMITTENT = 1 };

const char* envName(Env e);
const char* envShortName(Env e);

class Plume {
 public:
  void begin(Env env, float src_x_cm, float src_y_cm, float wind_from_deg, uint32_t seed);
  void step(float dt_s);
  float concentration(float x_cm, float y_cm);

  float srcX() const { return sx_; }
  float srcY() const { return sy_; }
  Env env() const { return env_; }

 private:
  struct Puff {
    float x, y, age;
  };

  Env env_ = Env::DIFFUSION;
  float sx_ = 0, sy_ = 0;
  float wind_dx_ = 0, wind_dy_ = 0;
  float emit_acc_ = 0.0f;
  std::vector<Puff> puffs_;
  std::mt19937 rng_;
  std::normal_distribution<float> gauss_{0.0f, 1.0f};
};

class Mq3Model {
 public:
  void begin(uint32_t seed);
  uint16_t sample(float concentration, float dt_s);
  float filtered() const { return y_; }

 private:
  float y_ = 0.0f;
  std::mt19937 rng_;
  std::normal_distribution<float> gauss_{0.0f, 1.0f};
};

}
