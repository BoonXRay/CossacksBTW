#ifndef MOTION_H
#define MOTION_H

#include "Path.h"

extern MotionField UnitsField;

int GetShipDanger( int x, int y );
int GetShipDanger1( int x, int y );

class OneObject;
void PerformMotion2( OneObject * OB );

#endif // MOTION_H
