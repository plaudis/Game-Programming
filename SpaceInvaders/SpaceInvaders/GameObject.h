#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

class GameObject
{
public:
	GameObject(int texture, float posX, float posY, float rot, float w, float h);
	GameObject(int spriteSheet, float posX, float posY, float dX, float dY, float rot, float uSprite, float vSprite, float w, float h);
	~GameObject();

	void Draw();
	void DrawSprite(float scale);

	float x;
	float y;
	float rotation;
	int textureID;
	float u;
	float v;
	float width;
	float height;

	float speed;
	float direction_x;
	float direction_y;
};

