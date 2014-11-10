#pragma once
#include <math.h>

class Vector
{
public:
	Vector();
	Vector(float x, float y, float z);
	float length();
	Vector normalize();
	float dot(Vector other);

	float x;
	float y;
	float z;
};

