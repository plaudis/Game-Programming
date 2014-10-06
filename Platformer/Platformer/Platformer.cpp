#include "Platformer.h"

// 60 FPS (1.0f/60.0f)
#define FIXED_TIMESTEP 0.0166666f
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

Platformer::Platformer() :keys(SDL_GetKeyboardState(NULL))
{
	Init();
	LoadMap();
	ResetGame();
}



Platformer::~Platformer()
{
	delete player;
	for (GLuint i = 0; i < enemies.size(); i++) { delete enemies[i]; }
	for (GLuint i = 0; i < map.size(); i++) { delete map[i]; }
	SDL_Quit();
}

void Platformer::Init()
{	
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

	glViewport(0, 0, 800, 600);
	glMatrixMode(GL_PROJECTION);
	glOrtho(-1.33, 1.33, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void Platformer::LoadMap(){
	fontTexture = LoadTexture("pixel_font.png");

	spriteSheet = LoadTexture("sprites.png");
	//int spriteSheet, float posX, float posY, float dX, float dY, float rot, float uSprite, float vSprite, float w, float h, float m, bool immovable, bool collidable
	player = new GameObject(spriteSheet, -1.0f, -0.8f, 0.0f, 0.0f, 0.0f, 141.0f / 256.0f, 145.0f / 256.0f, 66.0f / 256.0f, 92.0f / 256.0f, 1.0f, false, true);
	for (float i = -1.4f; i < 1.3f; i += 0.1f) {
		map.push_back(new GameObject(spriteSheet, i, -0.95f, 0.0f, 0.0f, 0.0f, 0.0f / 256.0f, 0.0f / 256.0f, 70.0f / 256.0f, 70.0f / 256.0f, 1.0f, true, true));
	}
	for (float i = -1.4f; i < 1.4f; i += 0.1f) {
		map.push_back(new GameObject(spriteSheet, i, 0.95f, 0.0f, 0.0f, 0.0f, 0.0f / 256.0f, 144.0f / 256.0f, 70.0f / 256.0f, 70.0f / 256.0f, 1.0f, true, true));
	}
	for (float i = -1.4f; i < 0.5f; i += 0.1f) {
		map.push_back(new GameObject(spriteSheet, i, -0.3f, 0.0f, 0.0f, 0.0f, 0.0f / 256.0f, 0.0f / 256.0f, 70.0f / 256.0f, 70.0f / 256.0f, 1.0f, true, true));
	}
	map.push_back(new GameObject(spriteSheet, 0.5, -0.3f, 0.0f, 0.0f, 0.0f, 72.0f / 256.0f, 0.0f / 256.0f, 70.0f / 256.0f, 70.0f / 256.0f, 1.0f, true, true));

	for (float i = -0.4f; i < 1.3f; i += 0.1f) {
		map.push_back(new GameObject(spriteSheet, i, 0.35f, 0.0f, 0.0f, 0.0f, 0.0f / 256.0f, 0.0f / 256.0f, 70.0f / 256.0f, 70.0f / 256.0f, 1.0f, true, true));
	}
	map.push_back(new GameObject(spriteSheet, -0.5f, 0.35f, 0.0f, 0.0f, 0.0f, 0.0f / 256.0f, 72.0f / 256.0f, 70.0f / 256.0f, 70.0f / 256.0f, 1.0f, true, true));

	for (float i = 0.0f; i < 1.40f; i += 0.1f) {
		map.push_back(new GameObject(spriteSheet, -1.3f, -0.3f+i, 0.0f, 0.0f, 0.0f, 0.0f / 256.0f, 144.0f / 256.0f, 70.0f / 256.0f, 70.0f / 256.0f, 1.0f, true, true));
		map.push_back(new GameObject(spriteSheet, 1.3f, -0.95f+i, 0.0f, 0.0f, 0.0f, 0.0f / 256.0f, 144.0f / 256.0f, 70.0f / 256.0f, 70.0f / 256.0f, 1.0f, true, true));
	}

}

void Platformer::ResetGame(){
	score = 0;
	alive = true;
	done = false;
	lastFrameTicks = 0.0f;
	timeLeftOver = 0.0f;
	timePassed = 0.0f;
	player->x=-1.0f;
	player->y = -0.8f;
	player->velocity_x = 0.0f;
	player->velocity_y = 0.0f;
	player->acceleration_x = 0.0f;
	player->acceleration_y = 0.0f;
	player->collidedBottom = false;
	player->collidedTop = false;
	player->collidedLeft = false;
	player->collidedRight = false;

	for (GLuint i = 0; i < enemies.size(); i++) { delete enemies[i]; }
	enemies.clear();
	
}

bool Platformer::UpdateAndRender()
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

void Platformer::FixedUpdate(){
	if (player->x - player->width*0.2f < -1.3f){ player->x=-1.3f + player->width*0.2f; }
	else if (player->x + player->width*0.2f > 1.2f && player->y>0.35f){ 
		score = 1;
	}



	player->velocity_y = lerp(player->velocity_y, 0.0f, FIXED_TIMESTEP * player->friction_y);
	player->velocity_y += player->acceleration_y * FIXED_TIMESTEP;
	player->y += player->velocity_y * FIXED_TIMESTEP;

	if (!player->collidedBottom){ player->acceleration_y = -2.0f; }//gravity


	for (GLuint i = 0; i < map.size(); i++) { 
		if (player->collidesWithY(map[i])){
			if (player->collidedBottom){ player->y = map[i]->y + map[i]->height*0.2f + player->height*0.2f; player->velocity_y = 0.0f; }
			else if (player->collidedTop){ player->y = map[i]->y - map[i]->height*0.2f - player->height*0.2f; player->velocity_y = 0.0f; player->collidedTop = false; }
		}
	}

	player->velocity_x = lerp(player->velocity_x, 0.0f, FIXED_TIMESTEP * player->friction_x);
	player->velocity_x += player->acceleration_x * FIXED_TIMESTEP;
	player->x += player->velocity_x * FIXED_TIMESTEP;

	for (GLuint i = 0; i < map.size(); i++) {
		if (player->collidesWithX(map[i])){
			if (player->collidedLeft){ player->x = map[i]->x + map[i]->width*0.2f + player->width*0.2f; player->velocity_x = 0.0f; player->collidedLeft = false; }
			else if (player->collidedRight){ player->x = map[i]->x - map[i]->width*0.2f - player->width*0.2f; player->velocity_x = 0.0f; player->collidedRight = false; }
		}
	}
	for (GLuint j = 0; j < enemies.size(); j++) {
		enemies[j]->velocity_x += enemies[j]->acceleration_x * FIXED_TIMESTEP;
		enemies[j]->x += enemies[j]->velocity_x * FIXED_TIMESTEP;
		for (GLuint i = 0; i < map.size(); i++) {
			if (enemies[j]->collidesWithX(map[i])){
				if (enemies[j]->collidedLeft){ enemies[j]->x = map[i]->x + map[i]->width*0.2f + enemies[j]->width*0.2f; enemies[j]->velocity_x = 0.5f; enemies[j]->collidedLeft = false; }
				else if (enemies[j]->collidedRight){ enemies[j]->x = map[i]->x - map[i]->width*0.2f - enemies[j]->width*0.2f; enemies[j]->velocity_x = -0.5f; enemies[j]->collidedRight = false; }
			}
		}
		if (enemies[j]->collidesWithX(player)){
			ResetGame();
			break;
		}

		enemies[j]->velocity_y = lerp(enemies[j]->velocity_y, 0.0f, FIXED_TIMESTEP * enemies[j]->friction_y);
		enemies[j]->velocity_y += enemies[j]->acceleration_y * FIXED_TIMESTEP;
		enemies[j]->y += enemies[j]->velocity_y * FIXED_TIMESTEP;
		if (!enemies[j]->collidedBottom){ enemies[j]->acceleration_y = -2.0f; }//gravity
		for (GLuint i = 0; i < map.size(); i++) {
			if (enemies[j]->collidesWithY(map[i])){
				if (enemies[j]->collidedBottom){ enemies[j]->y = map[i]->y + map[i]->height*0.2f + enemies[j]->height*0.2f; enemies[j]->velocity_y = 0.0f; enemies[j]->collidedBottom = false; }
				else if (enemies[j]->collidedTop){ enemies[j]->y = map[i]->y - map[i]->height*0.2f - enemies[j]->height*0.2f; enemies[j]->velocity_y = 0.0f; enemies[j]->collidedTop = false; }
			}
		}

		
		if (enemies[j]->collidesWithY(player)){
			ResetGame();
			break;
		}
	}

}

void Platformer::Update(float elapsed)
{
	timePassed += elapsed;

	if (timePassed > 2.0f){
		timePassed = 0.0f;
		enemies.push_back(new GameObject(spriteSheet, 1.0f, 0.8f, -0.5f, 0.0f, 0.0f, 196.0f / 256.0f, 94.0f / 256.0f, 50.0f / 256.0f, 28.0f / 256.0f, 1.0f, false, true));
	}

	if (keys[SDL_SCANCODE_LEFT]) {//move left
		player->acceleration_x = -1.0f;
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {//move right
		player->acceleration_x = 1.0f;
	}
	else { player->acceleration_x = 0.0f; }

	if (keys[SDL_SCANCODE_UP] && player->collidedBottom){//jump
		player->velocity_y = 2.5f;
		player->collidedBottom = false;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		/*else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				player->acceleration_y = 1.0f;
				player->collidedBottom = false;
			}
		}*/
	}
}


void Platformer::Render()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	if (score){ glTranslatef(-0.8f,0.0f,0.0f); PrintText(fontTexture, "YOU WIN!", 0.25f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f); }
	else{
		for (GLuint i = 0; i < map.size(); i++) { map[i]->DrawSprite(0.2f); }
		for (GLuint i = 0; i < enemies.size(); i++) { enemies[i]->DrawSprite(0.2f); }

		player->DrawSprite(0.2f);
	}
	SDL_GL_SwapWindow(displayWindow);
}
