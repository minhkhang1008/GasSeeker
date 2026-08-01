// ============================================================================
//  geometry.h - vai ham hinh hoc dung chung. Khong phu thuoc Arduino.
// ============================================================================
#pragma once
#include <cmath>
#include <cstdint>

#include "config.h"

namespace gs {

struct Pose {
  float x_cm = 0.0f;
  float y_cm = 0.0f;
  float heading_deg = 0.0f;  // nguoc chieu kim dong ho, 0 = huong +X
};

// Dua goc ve khoang (-180, 180].
inline float wrapDeg(float a) {
  while (a > 180.0f) a -= 360.0f;
  while (a <= -180.0f) a += 360.0f;
  return a;
}

inline float deg2rad(float d) { return d * 3.14159265358979f / 180.0f; }
inline float rad2deg(float r) { return r * 180.0f / 3.14159265358979f; }

inline float dist(float x1, float y1, float x2, float y2) {
  const float dx = x2 - x1, dy = y2 - y1;
  return std::sqrt(dx * dx + dy * dy);
}

inline float bearingDeg(float fx, float fy, float tx, float ty) {
  return rad2deg(std::atan2(ty - fy, tx - fx));
}

template <typename T>
inline T clampv(T v, T lo, T hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

// Diem (x,y) co nam trong san (da tru le an toan) khong?
inline bool insideArena(float x, float y) {
  const float m = cfg::ARENA_MARGIN_CM;
  return x >= m && x <= cfg::ARENA_W_CM - m && y >= m && y <= cfg::ARENA_H_CM - m;
}

// Diem den khi di tu (x,y) theo huong heading mot doan step.
inline void project(float x, float y, float heading_deg, float step_cm, float& ox, float& oy) {
  ox = x + step_cm * std::cos(deg2rad(heading_deg));
  oy = y + step_cm * std::sin(deg2rad(heading_deg));
}

inline int cellX(float x_cm) {
  int c = (int)(x_cm / cfg::CELL_CM);
  return clampv(c, 0, cfg::GRID_NX - 1);
}
inline int cellY(float y_cm) {
  int c = (int)(y_cm / cfg::CELL_CM);
  return clampv(c, 0, cfg::GRID_NY - 1);
}

}  // namespace gs
