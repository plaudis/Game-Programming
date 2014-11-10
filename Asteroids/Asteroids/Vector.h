#pragma once
#include <math.h>

class Vector
{
public:
	Vector();
	Vector(float xs, float ys, float zs);
	float length();
	Vector normalize();
	float dot(Vector other);

	float x;
	float y;
	float z;
};

