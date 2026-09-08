#pragma once
#include <Arduino.h>

#include "../core/geometry.h"

namespace hw {

void odomBegin();
void odomReset(float x_cm, float y_cm, float heading_deg);
void odomSetWheelDir(int left_dir, int right_dir);
void odomUpdate();

gs::Pose odomPose();
float odomTravelledCm();
bool odomUsingGyro();

void odomSegmentReset();
float odomSegmentCm();
float odomSegmentTurnDeg();

long odomTicksL();
long odomTicksR();
bool odomHasRightEncoder();

}
