#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <math.h>


class GameObject
{
public:
	GameObject(int spriteSheet, float posX, float posY, float wi, float hi, float dX, float dY, float rot, float uSprite, float vSprite, float wj, float hj, int hit, bool flip);
	~GameObject();
	void DrawSprite(float scale);

	bool collidesWithX(GameObject *other);
	bool collidesWithY(GameObject *other);

	float x;
	float y;
	float width;
	float height;
	float rotation;
	int textureID;
	float u;
	float v;
	float w;
	float h;

	float velocity_x;
	float velocity_y;
	float acceleration_x;
	float acceleration_y;
	float friction_x;
	float friction_y;
	int hits;
	int powerUp;

	bool flipped;
	bool collidedTop;
	bool collidedBottom;
	bool collidedLeft;
	bool collidedRight;
};