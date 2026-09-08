#pragma once
#include <cstdint>

namespace cfg {

constexpr char FW_VERSION[] = "GasSeeker-0.1";

#ifndef GS_ARENA_W_CM
#define GS_ARENA_W_CM 200.0f
#endif
#ifndef GS_ARENA_H_CM
#define GS_ARENA_H_CM 200.0f
#endif
#ifndef GS_CELL_CM
#define GS_CELL_CM 25.0f
#endif

constexpr float ARENA_W_CM = GS_ARENA_W_CM;
constexpr float ARENA_H_CM = GS_ARENA_H_CM;
constexpr float CELL_CM    = GS_CELL_CM;

constexpr int GRID_NX = (int)(ARENA_W_CM / CELL_CM);
constexpr int GRID_NY = (int)(ARENA_H_CM / CELL_CM);

constexpr float ARENA_MARGIN_CM = 10.0f;

constexpr float START_X_CM   = CELL_CM * 0.5f;
constexpr float START_Y_CM   = CELL_CM * 0.5f;
constexpr float START_HEADING_DEG = 0.0f;

constexpr float SUCCESS_RADIUS_CM = 30.0f;

#ifndef GS_MISSION_TIMEOUT_S
#define GS_MISSION_TIMEOUT_S 480
#endif
constexpr uint32_t MISSION_TIMEOUT_MS = GS_MISSION_TIMEOUT_S * 1000UL;

constexpr float WHEEL_DIAMETER_MM = 65.0f;
constexpr float WHEEL_BASE_MM     = 130.0f;
constexpr int   ENCODER_SLOTS     = 20;

constexpr int ENCODER_COUNT = 1;
constexpr int BUMPER_COUNT  = 1;

constexpr bool IMU_REQUIRED = (ENCODER_COUNT < 2);

constexpr float MM_PER_TICK = 3.14159265f * WHEEL_DIAMETER_MM / ENCODER_SLOTS;
constexpr float CM_PER_TICK = MM_PER_TICK / 10.0f;

constexpr float SENSOR_OFFSET_CM = 12.0f;

constexpr int PWM_MAX        = 255;
constexpr int PWM_DRIVE      = 150;
constexpr int PWM_TURN       = 130;
constexpr int PWM_MIN_MOVE   = 70;

constexpr float HEADING_KP        = 3.0f;
constexpr int   HEADING_MAX_CORR  = 60;

constexpr float TURN_TOLERANCE_DEG = 3.0f;
constexpr float TURN_KP           = 2.2f;

constexpr uint32_t BRAKE_MS = 250;

constexpr uint32_t MOTION_TIMEOUT_MS = 12000;

constexpr uint32_t GAS_SAMPLE_PERIOD_MS = 50;
constexpr int      GAS_MA_WINDOW        = 16;

constexpr uint32_t BASELINE_MS = 5000;

constexpr int16_t DETECT_DELTA    = 40;
constexpr int16_t STOP_HIGH_DELTA = 800;
constexpr int16_t PLATEAU_EPS     = 30;

constexpr uint32_t STOP_HOLD_MS = 6000;

constexpr uint32_t SNIFF_SETTLE_MS = 1500;
constexpr uint32_t SNIFF_AVG_MS    = 800;
constexpr uint32_t SNIFF_TOTAL_MS  = SNIFF_SETTLE_MS + SNIFF_AVG_MS;

constexpr float ADC_MAX_COUNT = 4095.0f;
constexpr float ADC_REF_MV    = 3100.0f;

constexpr float DIV_R_TOP_OHM = 10000.0f;
constexpr float DIV_R_BOT_OHM = 20000.0f;
constexpr float DIV_GAIN = DIV_R_BOT_OHM / (DIV_R_TOP_OHM + DIV_R_BOT_OHM);

constexpr float MQ3_VC_V   = 5.0f;
constexpr float MQ3_RL_OHM = 10000.0f;

constexpr float MQ3_RATIO_CLEAN_AIR = 60.0f;
constexpr float MQ3_R0_OHM = -1.0f;

constexpr float MQ3_CURVE_A = 315.2f;
constexpr float MQ3_CURVE_B = -1.865f;

constexpr float PPM_T1 = 100.0f;
constexpr float PPM_T2 = 500.0f;
constexpr float PPM_T3 = 1500.0f;


constexpr float EXH_STEP_CM     = CELL_CM;
constexpr float EXH_ROW_GAP_CM  = CELL_CM;

constexpr bool RETURN_TO_BEST = true;
constexpr float RETURN_MIN_DIST_CM = 12.0f;

constexpr float    GRAD_SWEEP_DEG   = 55.0f;
constexpr float    GRAD_STEP_CM     = 30.0f;
constexpr int      GRAD_STUCK_STEPS = 5;
constexpr float    GRAD_ESCAPE_DEG  = 120.0f;

constexpr int STALL_LIMIT_SNIFFS = 12;

constexpr int SEEK_STRIDE = 2;

constexpr float WIND_FROM_DEG = 0.0f;

constexpr float    SC_SURGE_STEP_CM   = 25.0f;
constexpr uint32_t SC_LOST_MS         = 7000;
constexpr float    SC_CAST_STEP_CM    = 15.0f;
constexpr float    SC_CAST_GROWTH     = 1.6f;
constexpr float    SC_CAST_MAX_CM     = 90.0f;
constexpr float    SC_SEEK_STEP_CM    = 30.0f;

constexpr uint32_t TELEMETRY_PERIOD_MS = 1000;

constexpr float LORA_FREQ_MHZ    = 923.0f;
constexpr float LORA_BW_KHZ      = 125.0f;
constexpr int   LORA_SF          = 9;
constexpr int   LORA_CR          = 5;
constexpr int   LORA_TX_POWER_DBM = 14;
constexpr uint8_t LORA_SYNC_WORD = 0x34;
constexpr int   LORA_PREAMBLE    = 8;

constexpr bool ENABLE_UPLINK = true;

namespace pin {

constexpr int MQ3_AO = 4;

constexpr int I2C_SDA = 8;
constexpr int I2C_SCL = 9;

constexpr int LORA_MOSI = 11;
constexpr int LORA_SCK  = 12;
constexpr int LORA_MISO = 13;
constexpr int LORA_NSS  = 10;
constexpr int LORA_RST  = 14;
constexpr int LORA_BUSY = 21;
constexpr int LORA_DIO1 = 47;

constexpr int MOT_STBY = 18;
constexpr int MOT_L_PWM = 5;
constexpr int MOT_L_IN1 = 6;
constexpr int MOT_L_IN2 = 7;
constexpr int MOT_R_PWM = 15;
constexpr int MOT_R_IN1 = 16;
constexpr int MOT_R_IN2 = 17;

constexpr int ENC_L = 1;
constexpr int ENC_R = 2;

constexpr int BUMP_L = 38;
constexpr int BUMP_R = 39;

constexpr int BTN = 0;
constexpr int RGB_LED = 48;
constexpr int BUZZER  = 42;
constexpr int LED_A = 40;
constexpr int LED_B = 41;

}

constexpr bool UI_USE_RGB_LED       = true;
constexpr bool UI_USE_DISCRETE_LEDS = false;
constexpr uint32_t BTN_LONG_PRESS_MS = 800;

constexpr uint32_t CONTROL_PERIOD_MS = 20;
constexpr uint32_t IMU_PERIOD_MS     = 10;

constexpr uint32_t MQ3_PREHEAT_MS = 5UL * 60UL * 1000UL;

}
