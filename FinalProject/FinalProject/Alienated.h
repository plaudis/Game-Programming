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
	float timeUpdate();
	void FixedUpdate();
	void Update(float elapsed);
	void Render();
	void RenderBackground();
	bool readHeader(std::ifstream &stream);
	bool readLayerData(std::ifstream &stream);
	bool readEntityData(std::ifstream &stream);
	int worldToTileX(float worldX);
	int worldToTileY(float worldY);

	void collideWithMapX(GameObject* obj);
	void collideWithMapY(GameObject* obj);

	void moveAround(GameObject* obj);
	void moveAround2P(GameObject* first, GameObject* second);
	void animatePlayer();
	void animateRival();
	void drawLives();

	void renderTileMap();

	void UpdateMenus();

	void RenderMenu();
	void RenderWin();
	void RenderLoss();
	void RenderPause();
	void RenderInstructions();
	void RenderAbout();

	int state;
	int selection;
	int score;
	bool alive;
	bool done;
	float lastFrameTicks;
	float timeLeftOver;
	float timePassed;
	float walkAnimationTime, walk2AnimationTime;
	float shakeValue;

	int mapWidth;
	int mapHeight;
	unsigned char** levelData;

	Mix_Music *music;
	Mix_Chunk *jumpSound, *winSound;

	SDL_Window* displayWindow;

	GLuint background;
	GLuint fontTexture;
	GLuint spriteSheet;
	GLuint laserSheet;
	GameObject* player;
	GameObject* rival;
	std::vector<GameObject*> playerLasers;
	std::vector<GameObject*> enemies;
	std::vector<GameObject*> deletedEnemies;

	const Uint8 *keys;
};

