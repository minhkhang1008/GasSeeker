#pragma once
#include <cstdint>

namespace sim {

constexpr float SRC_X_MIN_FRAC = 0.50f;
constexpr float SRC_X_MAX_FRAC = 0.90f;
constexpr float SRC_Y_MIN_FRAC = 0.20f;
constexpr float SRC_Y_MAX_FRAC = 0.80f;

constexpr float DIFF_R0_CM = 25.0f;
constexpr float DIFF_NOISE = 0.010f;

constexpr float WIND_SPEED_CMS   = 25.0f;
constexpr float PUFF_PERIOD_S    = 0.35f;
constexpr float PUFF_R0_CM       = 5.0f;
constexpr float PUFF_GROWTH_CM2S = 70.0f;
constexpr float PUFF_MASS        = 250.0f;
constexpr float PUFF_JITTER_CMS  = 14.0f;
constexpr float PUFF_MAX_AGE_S   = 22.0f;
constexpr int   PUFF_MAX         = 512;

constexpr float MQ3_TAU_RISE_S = 2.5f;
constexpr float MQ3_TAU_FALL_S = 8.0f;
constexpr float MQ3_BASE_ADC   = 450.0f;
constexpr float MQ3_GAIN_ADC   = 2600.0f;
constexpr float MQ3_EXP        = 0.75f;
constexpr float MQ3_NOISE_ADC  = 4.0f;

constexpr float DRIVE_SPEED_CMS = 18.0f;
constexpr float TURN_RATE_DPS   = 90.0f;
constexpr float DT_S            = 0.020f;

constexpr float ODO_DIST_SCALE_ERR = 0.015f;
constexpr float ODO_HEADING_DRIFT_DPS = 0.06f;
constexpr float ODO_TURN_ERR_DEG = 1.2f;

}
