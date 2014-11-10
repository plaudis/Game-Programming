#pragma once
#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

class Particle {
public:
	float posX;
	float posY;
	float vX;
	float vY;
	float lifetime;
};

class particleEmitter
{
public:
	particleEmitter(float x, float y);
	~particleEmitter();
	void Update(float elapsed);
	void Render();
	float posX;
	float posY;
	float maxLifetime;
	std::vector<Particle> particles;

};

