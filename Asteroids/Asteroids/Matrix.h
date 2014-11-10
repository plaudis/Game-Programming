#pragma once
#include <vector>
#include <Vector.h>

#define PI 3.14159265f

class Matrix
{
public:
	union {
		float m[4][4];
		float ml[16];
	};

	Matrix();
	~Matrix();

	void identity();
	void scale(float w, float h);
	void translate(float x, float y, float z);
	void rotate(float deg);

	Matrix inverse();
	Matrix operator * (const Matrix &m2);
	Vector operator * (const Vector &v2);
};


