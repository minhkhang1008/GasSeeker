#include "hw_gas.h"

#include "../core/config.h"

namespace hw {

void gasBegin() {
  analogReadResolution(12);
  // Atten 12 dB = toan dai ~0..3.1 V, phu hop voi dau ra mach chia ap
  // (5 V tren chan AO -> 3.33 V sau chia, thuc te MQ-3 hiem khi cham 5 V).
  analogSetPinAttenuation(cfg::pin::MQ3_AO, ADC_11db);
  pinMode(cfg::pin::MQ3_AO, INPUT);
}

uint16_t gasReadRaw() { return (uint16_t)analogRead(cfg::pin::MQ3_AO); }

float gasReadMv() { return (float)analogReadMilliVolts(cfg::pin::MQ3_AO); }

}  // namespace hw
