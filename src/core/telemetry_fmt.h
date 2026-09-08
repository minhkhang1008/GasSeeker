#pragma once
#include <cstddef>
#include <cstdint>

#include "irobot.h"

namespace gs {

struct TelemetrySample {
  uint32_t t_ms = 0;
  const char* algo = "?";
  const char* state = "?";
  uint16_t adc = 0;
  int16_t norm = 0;
  float ppm = 0.0f;
  AlarmLevel level = AlarmLevel::Safe;
  Pose pose;
  float dist_cm = 0.0f;
  int16_t best_norm = 0;
  bool finished = false;
};

size_t buildCsv(char* out, size_t n, const TelemetrySample& s);

bool prettyFromCsv(const char* csv, char* out, size_t n);

bool verifyChecksum(const char* line);

}
