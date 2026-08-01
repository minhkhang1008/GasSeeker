#include "plume.h"

#include <cmath>

#include "../core/config.h"
#include "../core/geometry.h"

namespace sim {

const char* envName(Env e) {
  return e == Env::DIFFUSION ? "KHUECH_TAN" : "DUT_QUANG";
}
const char* envShortName(Env e) { return e == Env::DIFFUSION ? "diff" : "inter"; }

// ---------------------------------------------------------------------------
void Plume::begin(Env env, float sx, float sy, float wind_from_deg, uint32_t seed) {
  env_ = env;
  sx_ = sx;
  sy_ = sy;
  // Gio THOI TU wind_from_deg -> vector di chuyen nguoc lai 180 do.
  const float dir = gs::deg2rad(wind_from_deg + 180.0f);
  wind_dx_ = std::cos(dir);
  wind_dy_ = std::sin(dir);
  emit_acc_ = 0.0f;
  puffs_.clear();
  puffs_.reserve(PUFF_MAX);
  rng_.seed(seed);
}

void Plume::step(float dt_s) {
  if (env_ != Env::INTERMITTENT) return;

  // 1. Nha bui moi.
  emit_acc_ += dt_s;
  while (emit_acc_ >= PUFF_PERIOD_S && (int)puffs_.size() < PUFF_MAX) {
    emit_acc_ -= PUFF_PERIOD_S;
    puffs_.push_back(Puff{sx_, sy_, 0.0f});
  }

  // 2. Gio cuon di + roi loan + gia di.
  const float jit = PUFF_JITTER_CMS * std::sqrt(dt_s);
  for (auto& p : puffs_) {
    p.x += wind_dx_ * WIND_SPEED_CMS * dt_s + gauss_(rng_) * jit;
    p.y += wind_dy_ * WIND_SPEED_CMS * dt_s + gauss_(rng_) * jit;
    p.age += dt_s;
  }

  // 3. Bo cac bui qua gia.
  size_t w = 0;
  for (size_t i = 0; i < puffs_.size(); ++i) {
    if (puffs_[i].age < PUFF_MAX_AGE_S) puffs_[w++] = puffs_[i];
  }
  puffs_.resize(w);
}

float Plume::concentration(float x, float y) {
  if (env_ == Env::DIFFUSION) {
    const float dx = x - sx_, dy = y - sy_;
    const float r2 = dx * dx + dy * dy;
    const float r02 = DIFF_R0_CM * DIFF_R0_CM;
    float c = r02 / (r02 + r2);
    c += gauss_(rng_) * DIFF_NOISE;
    return c < 0.0f ? 0.0f : c;
  }

  // Tong dong gop cua cac bui khi.
  float c = 0.0f;
  for (const auto& p : puffs_) {
    const float R2 = PUFF_R0_CM * PUFF_R0_CM + PUFF_GROWTH_CM2S * p.age;
    const float dx = x - p.x, dy = y - p.y;
    const float d2 = dx * dx + dy * dy;
    if (d2 > 9.0f * R2) continue;  // qua xa, bo qua cho nhanh
    c += (PUFF_MASS / (6.2831853f * R2)) * std::exp(-d2 / (2.0f * R2));
  }
  return c;
}

// ---------------------------------------------------------------------------
void Mq3Model::begin(uint32_t seed) {
  y_ = 0.0f;
  rng_.seed(seed);
}

uint16_t Mq3Model::sample(float c, float dt_s) {
  if (c < 0.0f) c = 0.0f;
  // Tre bac nhat bat doi xung: len nhanh, xuong cham.
  const float tau = (c > y_) ? MQ3_TAU_RISE_S : MQ3_TAU_FALL_S;
  const float a = dt_s / (tau + dt_s);
  y_ += a * (c - y_);

  float adc = MQ3_BASE_ADC + MQ3_GAIN_ADC * std::pow(y_ < 0.0f ? 0.0f : y_, MQ3_EXP);
  adc += gauss_(rng_) * MQ3_NOISE_ADC;
  if (adc < 0.0f) adc = 0.0f;
  if (adc > cfg::ADC_MAX_COUNT) adc = cfg::ADC_MAX_COUNT;
  return (uint16_t)(adc + 0.5f);
}

}  // namespace sim
