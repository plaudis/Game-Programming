#include "Vector.h"


Vector::Vector()
{
}

Vector::Vector(float x, float y, float z){
	x = x;
	y = y;
	z = z;
}

void Vector::normalize()
{
	x = 1;
	y = 1;
	z = 1;
}

float Vector::length()
{
	return sqrt(x*x + y*y);
}
