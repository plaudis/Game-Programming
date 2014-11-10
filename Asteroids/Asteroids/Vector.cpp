#include "Vector.h"


Vector::Vector()
{
}

Vector::Vector(float xs, float ys, float zs){
	x = xs;
	y = ys;
	z = zs;
}

Vector Vector::normalize()
{
	float l = length();
	return Vector(x/l,y/l,z/l);
}

float Vector::length()
{
	return sqrt(x*x + y*y + z*z);
}

float Vector::dot(Vector other)
{
	return ((other.x*x) + (other.y*y));
}
