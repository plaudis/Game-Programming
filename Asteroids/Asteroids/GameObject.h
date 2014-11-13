#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <math.h> 
#include <algorithm>
#include "Matrix.h"

#define PI 3.14159265f

class GameObject
{
public:
	GameObject(int sprite, float posX, float posY, float w, float h, float vx, float vy, float rot, float dR, float m);
	~GameObject();
	void DrawSprite();
	void buildMatrix();

	bool collidesWith(GameObject *other);

	Matrix matrix;

	float x;
	float y;
	float width;
	float height;
	float rotationDegrees;
	float rotationVelocity;
	float rotationAcceleration;
	int textureID;

	float velocity_x;
	float velocity_y;
	float acceleration;
	float mass;

	bool collidedTop;
	bool collidedBottom;
	bool collidedLeft;
	bool collidedRight;
};