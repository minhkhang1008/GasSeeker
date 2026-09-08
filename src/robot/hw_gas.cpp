#include "hw_gas.h"

#include "../core/config.h"

namespace hw {

void gasBegin() {
  analogReadResolution(12);
  analogSetPinAttenuation(cfg::pin::MQ3_AO, ADC_11db);
  pinMode(cfg::pin::MQ3_AO, INPUT);
}

uint16_t gasReadRaw() { return (uint16_t)analogRead(cfg::pin::MQ3_AO); }

float gasReadMv() { return (float)analogReadMilliVolts(cfg::pin::MQ3_AO); }

}
