#include "telemetry_fmt.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "config.h"

namespace gs {

static uint8_t xorChecksum(const char* begin, const char* end) {
  uint8_t c = 0;
  for (const char* p = begin; p < end; ++p) c ^= (uint8_t)*p;
  return c;
}

size_t buildCsv(char* out, size_t n, const TelemetrySample& s) {
  if (n < 32) return 0;

  const int cx = cellX(s.pose.x_cm);
  const int cy = cellY(s.pose.y_cm);

  int len = snprintf(out, n,
                     "$GS,%.1f,%s,%s,%u,%d,%.0f,%s,%.1f,%.1f,%.1f,%.1f,%d,%d,%d,%d",
                     s.t_ms / 1000.0f, s.algo, s.state, (unsigned)s.adc, (int)s.norm,
                     (double)s.ppm, alarmLevelName(s.level), (double)s.pose.x_cm,
                     (double)s.pose.y_cm, (double)s.pose.heading_deg, (double)s.dist_cm,
                     cx, cy, (int)s.best_norm, s.finished ? 1 : 0);
  if (len <= 0 || (size_t)len >= n - 4) return 0;

  const uint8_t ck = xorChecksum(out + 1, out + len);
  len += snprintf(out + len, n - (size_t)len, "*%02X", ck);
  return (size_t)len;
}

bool verifyChecksum(const char* line) {
  if (!line || line[0] != '$') return false;
  const char* star = strrchr(line, '*');
  if (!star || star < line + 1) return false;
  if (star[1] == '\0' || star[2] == '\0') return false;

  const uint8_t want = xorChecksum(line + 1, star);
  char buf[3] = {star[1], star[2], '\0'};
  const unsigned got = (unsigned)strtoul(buf, nullptr, 16);
  return (unsigned)want == got;
}

// Tach toi da 16 truong. Ghi de vao ban sao noi bo -> khong dung chuoi goc.
bool prettyFromCsv(const char* csv, char* out, size_t n) {
  char work[192];
  if (!csv || strlen(csv) >= sizeof(work)) return false;
  strncpy(work, csv, sizeof(work) - 1);
  work[sizeof(work) - 1] = '\0';

  char* star = strrchr(work, '*');
  if (star) *star = '\0';

  const char* f[16] = {nullptr};
  int nf = 0;
  char* p = work;
  while (nf < 16) {
    f[nf++] = p;
    char* c = strchr(p, ',');
    if (!c) break;
    *c = '\0';
    p = c + 1;
  }
  if (nf < 14) return false;
  if (strcmp(f[0], "$GS") != 0) return false;

  const int len = snprintf(out, n,
                           "t=%ss | ALGO=%s | ADC=%s | PPM=%s | LEVEL=%s | STATE=%s | CELL=(%s,%s)",
                           f[1], f[2], f[4], f[6], f[7], f[3], f[12], f[13]);
  return len > 0 && (size_t)len < n;
}

}  // namespace gs
