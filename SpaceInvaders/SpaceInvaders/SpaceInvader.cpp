#include "SpaceInvader.h"

using namespace std;

enum GameState { STATE_MENU=1, STATE_GAME=2, STATE_GAME_OVER=3 };

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	if (!surface){ return -1; }
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	SDL_FreeSurface(surface);

	return textureID;
}

void PrintText(GLuint fontTexture, string text, float size, float spacing, float r, float g, float b, float a) {
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float texture_size = 1.0 / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;
	vector<float> colorData;
	for (GLuint i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		colorData.insert(colorData.end(), { r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a });
		vertexData.insert(vertexData.end(), { ((size + spacing) * i) + (-0.5f * size), 0.5f * size, ((size + spacing) * i) +
			(-0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), 0.5f
			* size });
		texCoordData.insert(texCoordData.end(), { texture_x, texture_y, texture_x, texture_y + texture_size, texture_x +
			texture_size, texture_y + texture_size, texture_x + texture_size, texture_y });
	}
	glColorPointer(4, GL_FLOAT, 0, colorData.data());
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertexData.data());
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoordData.data());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_QUADS, 0, text.size() * 4);
	glDisableClientState(GL_COLOR_ARRAY);
}

SpaceInvader::SpaceInvader()
{
	Init();
	bool done = false;
	lastFrameTicks = 0.0f;
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
}



SpaceInvader::~SpaceInvader()
{
	delete ship;
	for (GLuint i = 0; i < playerBullets.size(); i++) { delete playerBullets[i]; }
	for (GLuint i = 0; i < enemyBullets.size(); i++) { delete enemyBullets[i]; }
	for (GLuint i = 0; i < enemies.size(); i++) { delete enemies[i]; }
	SDL_Quit();
}

void SpaceInvader::Init()
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

	glViewport(0, 0, 800, 600);
	glMatrixMode(GL_PROJECTION);
	glOrtho(-1.33, 1.33, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	

	fontTexture = LoadTexture("pixel_font.png");

	spriteSheet = LoadTexture("sprites.png");

	/*GLuint shipSprite = LoadTexture("ship.png");
	ship = new GameObject(shipSprite, 0.0f, -0.9f, 0.0f, 0.2f, 0.2f);*/
	//int spriteSheet, float posX, float posY, float rot, float uSprite, float vSprite, float w, float h
	ship = new GameObject(spriteSheet, 0.0f, -0.9f,0.0f,0.0f, 0.0f, 0.0f/256.0f, 156.0f/512.0f, 112.0f/256.0f, 75.0f/512.0f);
	enemies.push_back(new GameObject(spriteSheet, -0.8f, 0.5f, 0.0f, 0.0f, 0.0f, 114.0f / 256.0f, 0.0f / 512.0f, 93.0f / 256.0f, 84.0f / 512.0f));
	enemies.push_back(new GameObject(spriteSheet, -0.4f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f / 256.0f, 311.0f / 512.0f, 104.0f / 256.0f, 84.0f / 512.0f));
	enemies.push_back(new GameObject(spriteSheet, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f / 256.0f, 397.0f / 512.0f, 103.0f / 256.0f, 84.0f / 512.0f));
	enemies.push_back(new GameObject(spriteSheet, 0.4f, 0.5f, 0.0f, 0.0f, 0.0f, 114.0f / 256.0f, 86.0f / 512.0f, 82.0f / 256.0f, 84.0f / 512.0f));
	enemies.push_back(new GameObject(spriteSheet, 0.8f, 0.5f, 0.0f, 0.0f, 0.0f, 106.0f / 256.0f, 311.0f / 512.0f, 97.0f / 256.0f, 84.0f / 512.0f));

	state = 1;
	score = 0;
}

bool SpaceInvader::UpdateAndRender()
{
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;

	Update(elapsed);
	Render();
	return done;
}

void SpaceInvader::UpdateBullets(float elapsed, vector<GameObject*>& bullets){
	for (GLuint i = 0; i < bullets.size(); i++) {
		if (bullets[i]->y > 1.1 || bullets[i]->y < -1.1){  
			if (bullets[i]){
				delete bullets[i];
				bullets.erase(bullets.begin() + i);
			}
		}
		else{
			bullets[i]->y += bullets[i]->direction_y*bullets[i]->speed*elapsed;
		}
		
	}
}

void SpaceInvader::UpdateEnemiesAndScore(float elapsed, vector<GameObject*>& enemies, vector<GameObject*>& bullets){
	for (GLuint i = 0; i < enemies.size(); i++) {
		for (GLuint j = 0; j < bullets.size(); j++) {
			if ((enemies[i]->x + (enemies[i]->width*0.3f) > bullets[j]->x) &&
				(enemies[i]->x - (enemies[i]->width*0.3f) < bullets[j]->x) &&
				(enemies[i]->y + (enemies[i]->height*0.3f) > bullets[j]->y) &&
				(enemies[i]->y - (enemies[i]->height*0.3f) < bullets[j]->y)){
				if (bullets[j]){
					delete bullets[j];
					bullets.erase(bullets.begin() + j);
				}
				if (enemies[i]){
					delete enemies[i];
					enemies.erase(enemies.begin() + i);
					score += 100;
				}
				
				if (!enemies.size()) state = 3;
			}
		}
	}
}

void SpaceInvader::Update(float elapsed)
{
	UpdateBullets(elapsed, playerBullets);
	UpdateBullets(elapsed, enemyBullets);
	UpdateEnemiesAndScore(elapsed, enemies, playerBullets);

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == 1){
				if (state == 2){
					playerBullets.push_back(new GameObject(spriteSheet, ship->x, -0.8f, 0.0f, 1.0f, 0.0f, 158.0f / 256.0f, 211.0f / 512.0f, 13.0f / 256.0f, 57.0f / 512.0f));
				}
				else if (state == 3){
					enemies.push_back(new GameObject(spriteSheet, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 114.0f / 256.0f, 0.0f / 512.0f, 93.0f / 256.0f, 84.0f / 512.0f)); 
					score = 0;
					state = 2;
				}
				else state = 2;
			}
		}
		else if (event.type == SDL_MOUSEMOTION) {
			ship->x = (((float)event.motion.x / 800.0f) * 2.666f) - 1.333f;//the new x position from mouse
			if (ship->x > 1.2f){ ship->x = 1.2f; }
			else if (ship->x < -1.2f){ ship->x = -1.2f; }
		}
	}
}


void SpaceInvader::Render()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	switch (state) {
	case STATE_MENU:
		RenderMenu();
		break;
	case STATE_GAME:
		RenderGame();
		break;
	case STATE_GAME_OVER:
		RenderWin();
		break;
	}
	SDL_GL_SwapWindow(displayWindow);
}

void SpaceInvader::RenderMenu(){
	glLoadIdentity();
	glTranslatef(-0.8f, 0.5f, 0.0);
	PrintText(fontTexture, "SpaceInvaders",0.13f,0.0f,1.0f,1.0f,1.0f,1.0f);
	glLoadIdentity();
	glTranslatef(-0.3f, -0.5f, 0.0);
	PrintText(fontTexture, "Click To Play", 0.05f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
}

void SpaceInvader::RenderGame(){
	glLoadIdentity();
	glTranslatef(-1.2f, 0.9f, 0.0);
	PrintText(fontTexture, to_string(score),0.05f,0.0f,1.0f,1.0f,1.0f,1.0f);
	glLoadIdentity();
	ship->DrawSprite(0.3f);
	for (GLuint i = 0; i < enemies.size(); i++) {
		enemies[i]->DrawSprite(0.3f);
	}
	for (GLuint i = 0; i < playerBullets.size(); i++) {
		playerBullets[i]->DrawSprite(0.3f);
	}
}

void SpaceInvader::RenderWin(){
	glLoadIdentity();
	glTranslatef(-0.9f, 0.5f, 0.0);
	PrintText(fontTexture, "YOU WIN!", 0.25f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glTranslatef(0.45f, -1.0f, 0.0);
	PrintText(fontTexture, "Click To Play Again", 0.05f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
}

void SpaceInvader::RenderLoss(){
	glLoadIdentity();
	glTranslatef(-0.8f, 0.5f, 0.0);
	PrintText(fontTexture, "YOU LOSE!", 0.13f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(-0.3f, -0.5f, 0.0);
	PrintText(fontTexture, "Click To Play Again", 0.05f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
}