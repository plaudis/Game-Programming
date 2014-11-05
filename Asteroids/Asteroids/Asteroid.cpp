#include "Asteroid.h"

#define FIXED_TIMESTEP 0.0166666f // 60 FPS (1.0f/60.0f)
#define MAX_TIMESTEPS 6

#define OBJECT_SIZE 0.2f

using namespace std;

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

Asteroid::Asteroid()
{
	Init();
	//LoadMap();
	ResetGame();
}


Asteroid::~Asteroid()
{
	//delete player;
	//for (GLuint i = 0; i < enemies.size(); i++) { delete enemies[i]; }
	SDL_Quit();
}

void Asteroid::Init()
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("PlatformAdventure", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	glViewport(0, 0, 800, 600);
	glMatrixMode(GL_PROJECTION);
	//glOrtho(-1.33 * 10, 1.33 * 10, -1.0 * 10, 1.0 * 10, -1.0, 1.0);
	glOrtho(-1.33 , 1.33 , -1.0 , 1.0 , -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	//spriteSheet = LoadTexture("arne_sprites.png");
	GLuint playerSprite = LoadTexture("player.png");
	//int sprite, float posX, float posY, float w, float h, float v, float rot, float dR, float m
	player = new GameObject(playerSprite, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

}

void Asteroid::ResetGame(){
	score = 0;
	alive = true;
	done = false;
	lastFrameTicks = 0.0f;
	timeLeftOver = 0.0f;
	timePassed = 0.0f;
	player->x = 0.0f;
	player->y = 0.0f;
	player->velocity = 0.0f;
	player->acceleration = 0.0f;
	player->collidedBottom = false;
	player->collidedTop = false;
	player->collidedLeft = false;
	player->collidedRight = false;
}

bool Asteroid::UpdateAndRender()
{
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;

	float fixedElapsed = elapsed + timeLeftOver;
	if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
		fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
	}
	while (fixedElapsed >= FIXED_TIMESTEP) {
		fixedElapsed -= FIXED_TIMESTEP;
		FixedUpdate();
	}
	timeLeftOver = fixedElapsed;

	Update(elapsed);
	Render();
	return done;
}

float lerp(float v0, float v1, float t) {
	return (1.0f - t)*v0 + t*v1;
}

void Asteroid::FixedUpdate(){

	////Y movement
	//player->velocity_y = lerp(player->velocity_y, 0.0f, FIXED_TIMESTEP * player->friction_y);
	//player->velocity_y += player->acceleration_y * FIXED_TIMESTEP;
	//player->y += player->velocity_y * FIXED_TIMESTEP;

	//if (!player->collidedBottom){ player->acceleration_y = -3.0f; }//gravity

	////collision detetection Y
	//collideWithMapY(player);

	////X movement
	//player->velocity_x = lerp(player->velocity_x, 0.0f, FIXED_TIMESTEP * player->friction_x);
	//player->velocity_x += player->acceleration_x * FIXED_TIMESTEP;
	//player->x += player->velocity_x * FIXED_TIMESTEP;

	////collision detection X
	//collideWithMapX(player);

	//for (GLuint j = 0; j < enemies.size(); j++) {
	//	enemies[j]->velocity_x += enemies[j]->acceleration_x * FIXED_TIMESTEP;
	//	enemies[j]->x += enemies[j]->velocity_x * FIXED_TIMESTEP;

	//	collideWithMapX(enemies[j]);

	//	if (enemies[j]->collidesWithX(player)){
	//		ResetGame();
	//		break;
	//	}

	//	enemies[j]->velocity_y = lerp(enemies[j]->velocity_y, 0.0f, FIXED_TIMESTEP * enemies[j]->friction_y);
	//	enemies[j]->velocity_y += enemies[j]->acceleration_y * FIXED_TIMESTEP;
	//	enemies[j]->y += enemies[j]->velocity_y * FIXED_TIMESTEP;

	//	if (!enemies[j]->collidedBottom){ enemies[j]->acceleration_y = -2.0f; }//gravity

	//	collideWithMapY(enemies[j]);

	//	if (enemies[j]->collidesWithY(player)){
	//		ResetGame();
	//		break;
	//	}
	//}

}

void Asteroid::Update(float elapsed)
{
	timePassed += elapsed;

	if (keys[SDL_SCANCODE_UP]) {//move forward
		player->acceleration = 2.0f;
	}
	else if (keys[SDL_SCANCODE_DOWN]) {//move back
		player->acceleration = -2.0f;
	}
	else if (keys[SDL_SCANCODE_LEFT]) {//rotate counter-clockwise
		player->rotationAcceleration = -1.0;
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {//rotate clockwise
		player->rotationAcceleration = 1.0;
	}
	else {
		player->acceleration = 0.0f; 
		player->rotationAcceleration = 0.0f;
	}


	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
}

void Asteroid::Render()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		//if (player->y>-4.4 && player->x>2 * 1.33)
		//	glTranslatef(-player->x, -player->y, 0.0f); //scrolling
	
		//renderMap();
	//for (GLuint i = 0; i < enemies.size(); i++) { enemies[i]->DrawSprite(OBJECT_SIZE); }
	//player->DrawSprite(OBJECT_SIZE);
	glPopMatrix();
	SDL_GL_SwapWindow(displayWindow);
}