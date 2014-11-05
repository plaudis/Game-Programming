#pragma once
#include <vector>
#include <Vector.h>

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
	Matrix inverse();
	Matrix operator * (const Matrix &m2);
	Vector operator * (const Vector &v2);
};


