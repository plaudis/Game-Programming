#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include "GameObject.h"

class Alienated
{
public:
	Alienated();
	~Alienated();

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

	void collideWithMapX(GameObject* obj);
	void collideWithMapY(GameObject* obj);

	void renderTileMap();

	int score;
	bool alive;
	bool done;
	float lastFrameTicks;
	float timeLeftOver;
	float timePassed;
	float walkAnimationTime;

	int mapWidth;
	int mapHeight;
	unsigned char** levelData;

	Mix_Music *music;
	Mix_Chunk *jumpSound, *winSound;

	SDL_Window* displayWindow;

	GLuint fontTexture;
	GLuint spriteSheet;
	GameObject* player;
	std::vector<GameObject*> playerLasers;
	std::vector<GameObject*> enemies;
	std::vector<GameObject*> deletedEnemies;


	//particleEmitter * particleSource;

	float shakeValue;

	const Uint8 *keys;
};

