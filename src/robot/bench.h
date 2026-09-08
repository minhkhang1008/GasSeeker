#pragma once
#include <Arduino.h>

class RobotIO;

namespace bench {

void begin(RobotIO* io);
bool handleCommand(const char* cmd);
bool active();
void update();
void abort();
void printHelp();

}
