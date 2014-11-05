#pragma once
#include <Asteroid.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include "GameObject.h"

class Asteroid
{
public:
	Asteroid();
	~Asteroid();
	bool UpdateAndRender();
private:
	void Init();
	void LoadMap();
	void LoadTileMap();
	void ResetGame();
	void FixedUpdate();
	void Update(float elapsed);
	void Render();

	bool done;
	
	int score;
	bool alive;
	
	float lastFrameTicks;
	float timeLeftOver;
	float timePassed;

	

	//GLuint spriteSheet;
	GameObject* player;
	//std::vector<GameObject*> enemies;
	SDL_Window* displayWindow;
	const Uint8 *keys;
};