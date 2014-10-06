#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include <string>
#include "GameObject.h"

class Platformer
{
public:
	Platformer();
	~Platformer();
	bool UpdateAndRender();


private:
	void Init();
	void LoadMap();
	void ResetGame();
	void FixedUpdate();
	void Update(float elapsed);
	void Render();

	int score;
	bool alive;
	bool done;
	float lastFrameTicks;
	float timeLeftOver;
	float timePassed;

	SDL_Window* displayWindow;

	GLuint fontTexture;
	GLuint spriteSheet;
	GameObject* player;
	std::vector<GameObject*> map;
	std::vector<GameObject*> enemies;

	const Uint8 *keys;
};

