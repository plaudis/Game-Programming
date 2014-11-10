#include "Asteroid.h"

#define FIXED_TIMESTEP 0.0166666f // 60 FPS (1.0f/60.0f)
#define MAX_TIMESTEPS 6

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

Asteroid::Asteroid() :keys(SDL_GetKeyboardState(NULL))
{
	Init();
	ResetGame();
}


Asteroid::~Asteroid()
{
	delete player;
	for (GLuint i = 0; i < asteroids.size(); i++) { delete asteroids[i]; }
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
	playerSprite = LoadTexture("player.png");
	asteroidSprite = LoadTexture("asteroid.png");
	//int sprite, float posX, float posY, float w, float h, float vx, float vy, float rot, float dR, float m
	player = new GameObject(playerSprite, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

}

void Asteroid::LoadAsteroids(){
	for (GLuint i = 0; i < asteroids.size(); i++) { delete asteroids[i]; }
	for (GLuint i = 0; i<1; i++) { asteroids.push_back(new GameObject(asteroidSprite, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 180.0f, 1.0f)); }
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
	player->velocity_x = 0.0f;
	player->velocity_y = 0.0f;
	player->acceleration = 0.0f;
	player->collidedBottom = false;
	player->collidedTop = false;
	player->collidedLeft = false;
	player->collidedRight = false;
	LoadAsteroids();
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
	//Rotation
	player->rotationVelocity = lerp(player->rotationVelocity, 0.0f, FIXED_TIMESTEP * player->mass);
	player->rotationVelocity += player->rotationAcceleration * FIXED_TIMESTEP;
	player->rotationDegrees += player->rotationVelocity * FIXED_TIMESTEP;

	if (player->rotationDegrees > 360.0f)player->rotationDegrees -= 360.0f;		//limit the angle
	else if (player->rotationDegrees < -360.0f)player->rotationDegrees += 360.0f;
	

	////Y movement
	player->velocity_y = lerp(player->velocity_y, 0.0f, FIXED_TIMESTEP * player->mass);
	player->velocity_y += player->acceleration * cos(player->rotationDegrees*PI / 180.0f) * FIXED_TIMESTEP;
	player->y += player->velocity_y * FIXED_TIMESTEP;

	////X movement
	player->velocity_x = lerp(player->velocity_x, 0.0f, FIXED_TIMESTEP * player->mass);
	player->velocity_x += player->acceleration * -sin(player->rotationDegrees*PI / 180.0f) * FIXED_TIMESTEP;
	player->x += player->velocity_x * FIXED_TIMESTEP;

	//limit the movement
	if (player->x>1.33f)player->x = 1.33f;
	else if (player->x<-1.33f)player->x = -1.33f;

	if (player->y>1.0f)player->y = 1.0f;
	else if (player->y<-1.0f)player->y = -1.0f;

	//Move asteroids
	for (GLuint j = 0; j < asteroids.size(); j++) {
		asteroids[j]->x += asteroids[j]->velocity_x * FIXED_TIMESTEP;
		asteroids[j]->y += asteroids[j]->velocity_y * FIXED_TIMESTEP;
		asteroids[j]->rotationDegrees += asteroids[j]->rotationVelocity * FIXED_TIMESTEP;
		//collision detection
		if (player->collidesWith(asteroids[j]) && asteroids[j]->collidesWith(player)){ asteroids[j]->x = -asteroids[j]->x; }
	}
	////collision detection 

	//	if (enemies[j]->collidesWithX(player)){
	//		ResetGame();
	//		break;
	//	}
}

void Asteroid::Update(float elapsed)
{
	timePassed += elapsed;

	if (keys[SDL_SCANCODE_UP] && keys[SDL_SCANCODE_DOWN]) {//Don't move
		player->acceleration = 0.0f;
	}
	else if (keys[SDL_SCANCODE_UP]) {//move forward
		player->acceleration = 2.0f;
	}
	else if (keys[SDL_SCANCODE_DOWN]) {//move back
		player->acceleration = -2.0f;
	}
	else {
		player->acceleration = 0.0f;
	}
	
	if (keys[SDL_SCANCODE_LEFT] && keys[SDL_SCANCODE_RIGHT]) {//Don't rotate
		player->rotationAcceleration = 0.0;
	}
	else if (keys[SDL_SCANCODE_LEFT]) {//rotate counter-clockwise
		player->rotationAcceleration = 180.0;
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {//rotate clockwise
		player->rotationAcceleration = -180.0;
	}
	else {
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
	for (GLuint i = 0; i < asteroids.size(); i++) { asteroids[i]->DrawSprite(); }
	player->DrawSprite();
	glPopMatrix();
	SDL_GL_SwapWindow(displayWindow);
}