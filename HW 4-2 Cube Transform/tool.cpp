#include <iostream>
#include <cmath>
#include "tool.h"
#include <gl/freeglut.h>

double Length(double x, double y, double z)
{
	return std::sqrt(x*x + y*y + z*z);
}
