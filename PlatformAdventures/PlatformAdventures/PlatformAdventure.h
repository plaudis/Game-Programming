#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include "GameObject.h"

class PlatformAdventure
{
public:
	PlatformAdventure();
	~PlatformAdventure();

	bool UpdateAndRender();


private:
	void Init();
	void LoadMap();
	void LoadTileMap();
	void ResetGame();
	void FixedUpdate();
	void Update(float elapsed);
	void Render();
	bool readHeader(std::ifstream &stream);
	bool readLayerData(std::ifstream &stream);
	bool readEntityData(std::ifstream &stream);
	int worldToTileX(float worldX);
	int worldToTileY(float worldY);

	void renderMap();

	int score;
	bool alive;
	bool done;
	float lastFrameTicks;
	float timeLeftOver;
	float timePassed;

	int mapWidth;
	int mapHeight;
	unsigned char** levelData;

	SDL_Window* displayWindow;

	GLuint fontTexture;
	GLuint spriteSheet;
	GameObject* player;
	std::vector<GameObject*> enemies;

	const Uint8 *keys;
};

