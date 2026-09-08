#pragma once
#include <Arduino.h>

namespace radiolink {

bool begin(bool verbose = true);
bool ok();
void poll();

bool send(const char* line);
bool busy();

bool receive(char* buf, size_t n, float* rssi = nullptr, float* snr = nullptr);

const char* lastError();

}
