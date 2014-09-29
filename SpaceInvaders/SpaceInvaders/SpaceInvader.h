#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <GameObject.h>
#include <vector>
#include <string>


class SpaceInvader
{
public:
	SpaceInvader();
	~SpaceInvader();

	void Init();
	bool UpdateAndRender();
	void Render();
	void Update(float elapsed);

private:
	int state;
	int score;
	int lives;
	bool done;
	float lastFrameTicks;
	SDL_Window* displayWindow;

	GLuint fontTexture;
	GLuint spriteSheet;
	GameObject* ship;
	std::vector<GameObject*> enemies;
	std::vector<GameObject*> playerBullets;
	std::vector<GameObject*> enemyBullets;

	void UpdateBullets(float elapsed, std::vector<GameObject*>& bullets);
	void UpdateEnemiesAndScore(float elapsed, std::vector<GameObject*>& enemies, std::vector<GameObject*>& bullets);

	void UpdateMenu();
	void UpdateGame();

	void RenderMenu();
	void RenderGame();
	void RenderWin();
	void RenderLoss();
};

