#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>


class GameObject
{
public:
	GameObject(int spriteSheet, float posX, float posY, float dX, float dY, float rot, float uSprite, float vSprite, float w, float h, float m, bool immovable, bool collidable);
	~GameObject();
	void DrawSprite(float scale);

	bool collidesWithX(GameObject *other);
	bool collidesWithY(GameObject *other);

	float x;
	float y;
	float rotation;
	int textureID;
	float u;
	float v;
	float width;
	float height;

	float velocity_x;
	float velocity_y;
	float acceleration_x;
	float acceleration_y;
	float friction_x;
	float friction_y;
	float mass;

	bool isStatic;
	bool enableCollisions;
	bool collidedTop;
	bool collidedBottom;
	bool collidedLeft;
	bool collidedRight;
};