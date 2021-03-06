#include "Alienated.h"

#define FIXED_TIMESTEP 0.0166666f // 60 FPS (1.0f/60.0f)
#define MAX_TIMESTEPS 6
#define SPRITE_COUNT_X 12
#define SPRITE_COUNT_Y 13
#define TILE_SIZE 0.2f
#define OBJECT_SIZE 0.2f
#define WORLD_OFFSET_X 0
#define WORLD_OFFSET_Y 0
#define MAX_HITS 5

using namespace std;

enum GameState { STATE_MENU = 1, STATE_PVP = 2, STATE_SINGLE_PLAYER = 3, STATE_GAME_PAUSED = 4, STATE_GAME_OVER = 5, STATE_INSTRUCTIONS = 6, STATE_ABOUT = 7};

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	if (!surface){ return -1; }
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	if (image_path == "tiles_spritesheet.png" || image_path == "pixel_font.png"){
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
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
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

Alienated::Alienated() :keys(SDL_GetKeyboardState(NULL))
{
	Init();
	ResetGame();
}

Alienated::~Alienated()
{
	delete player;
	for (GLuint i = 0; i < enemies.size(); i++) { delete enemies[i]; }
	for (GLuint i = 0; i < playerLasers.size(); i++) { delete playerLasers[i]; }
	for (GLuint i = 0; i < deletedEnemies.size(); i++) { delete deletedEnemies[i]; }
	Mix_FreeChunk(jumpSound);
	Mix_FreeChunk(winSound);
	Mix_FreeChunk(laser);
	Mix_FreeChunk(plasmaReload);
	Mix_FreeMusic(music);
	Mix_FreeChunk(land);
	Mix_FreeChunk(die);
	Mix_FreeChunk(menu);
	Mix_FreeChunk(select);
	SDL_Quit();
}

void Alienated::Init()
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Alienated", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	glViewport(0, 0, 800, 600);
	glMatrixMode(GL_PROJECTION);
	glOrtho(-1.33 * 2, 1.33 * 2, -1.0 * 2, 1.0 * 2, -1.0, 1.0);
	//glOrtho(-1.33 * 10, 1.33 * 10, -1.0 * 10, 1.0 * 10, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	title = LoadTexture("title.png");
	background = LoadTexture("space.png");
	//background = LoadTexture("outer_space.png");
	fontTexture = LoadTexture("font.png");
	spriteSheet = LoadTexture("tiles_spritesheet.png");
	laserSheet = LoadTexture("lasers.png");
	enemySheet = LoadTexture("enemies.png");
	playerHUD = LoadTexture("hud_p1.png");
	rivalHUD = LoadTexture("hud_p2.png");
	jumpSound = Mix_LoadWAV("jumpSound.wav");
	winSound = Mix_LoadWAV("triumph.wav");
	laser = Mix_LoadWAV("laser.wav");
	plasmaReload = Mix_LoadWAV("plasma.wav");
	land = Mix_LoadWAV("land.wav");
	die = Mix_LoadWAV("death.wav"); 
	menu = Mix_LoadWAV("menu.wav");
	select = Mix_LoadWAV("select.wav");
	music = Mix_LoadMUS("music.mp3");
	Mix_PlayMusic(music, -1);
	GLuint playerSpriteSheet = LoadTexture("p1_spritesheet.png");
	player = new GameObject(playerSpriteSheet, 0.0f, 0.0f, 0.66f, 0.92f, 0.0f, 0.0f, 0.0f, 0.0f / 508.0f, 196.0f / 288.0f, 66.0f / 508.0f, 92.0f / 288.0f, 0, true);
	GLuint rivalSpriteSheet = LoadTexture("p2_spritesheet.png");
	rival = new GameObject(rivalSpriteSheet, 0.0f, 0.0f, 0.66f, 0.92f, 0.0f, 0.0f, 0.0f, 0.0f / 494.0f, 190.0f / 282.0f, 66.0f / 494.0f, 92.0f / 282.0f, 0, false);
	playerScore = 0;
	rivalScore = 0;
}

void Alienated::LoadTileMap(string file){
	ifstream infile(file);
	string line;
	while (getline(infile, line)) {
		if (line == "[header]") {
			if (!readHeader(infile)) {
				return;
			}
		}
		else if (line == "[layer]") {
			readLayerData(infile);
		}
		else if (line == "[Players]") {
			readEntityData(infile);
		}
		else if (line == "[Plasma]") {
			readEntityData(infile);
		}
		else if (line == "[Enemies]") {
			readEntityData(infile);
		}
	}
}

bool Alienated::readHeader(std::ifstream &stream) {
	string line;
	mapWidth = -1;
	mapHeight = -1;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height"){
			mapHeight = atoi(value.c_str());
		}
	}
	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else { // allocate our map data
		levelData = new unsigned char*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}

bool Alienated::readLayerData(std::ifstream &stream) {
	string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < mapHeight; y++) {
				getline(stream, line);
				istringstream lineStream(line);
				string tile;
				for (int x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					if (val > 0) {
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = val;
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}

bool Alienated::readEntityData(std::ifstream &stream) {
	string line;
	string type;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			float placeX = atoi(xPosition.c_str()) / 70 * TILE_SIZE;
			float placeY = atoi(yPosition.c_str()) / 70 * -TILE_SIZE;
			if (type == "player"){ player->x = placeX; player->y = placeY; }
			else if (type == "player2"){ rival->x = placeX; rival->y = placeY; }
			else if (type == "plasma"){ plasmaLocations.push_back(new GameObject(LoadTexture("plasma.png"), placeX, placeY, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f , 0.0f, 1.0f, 1.0f, 0, false)); }
			else if (type == "enemy") {
				enemies.push_back(new GameObject(enemySheet, placeX, placeY, 0.72f, 0.51f, -1.5f, 0.0f, 0.0f, //width,height,dx,dy,rot
					79.0f/256, //u
					0.0f/64, //v
					72.0f/256, //w
					51.0f/64, //h
					1, false));
			}	
		}
	}
	return true;
}

void Alienated::ResetGame(){
	state = 1;
	selection = 0;
	score = 0;
	alive = true;
	done = false;
	lastFrameTicks = 0.0f;
	timeLeftOver = 0.0f;
	timePassed = 0.0f;
	srand(time(nullptr));
	for (GLuint i = 0; i < plasmaLocations.size(); i++) { delete plasmaLocations[i]; }
	plasmaLocations.clear();
	LoadTileMap("arena.txt");
	plasmaLocations[rand() % plasmaLocations.size()]->flipped = true;
	GLuint next = rand() % plasmaLocations.size();
	while (plasmaLocations[next]->flipped)next = rand() % plasmaLocations.size();
	plasmaLocations[next]->flipped = true;
	walkAnimationTime = 0.0f;
	player->hits = 0;
	player->bullets = 3;
	player->velocity_x = 0.0f;
	player->velocity_y = 0.0f;
	player->acceleration_x = 0.0f;
	player->acceleration_y = 0.0f;
	player->collidedBottom = false;
	player->collidedTop = false;
	player->collidedLeft = false;
	player->collidedRight = false;
	walk2AnimationTime = 0.0f;
	rival->hits = 0;
	rival->bullets = 3;
	rival->velocity_x = 0.0f;
	rival->velocity_y = 0.0f;
	rival->acceleration_x = 0.0f;
	rival->acceleration_y = 0.0f;
	rival->collidedBottom = false;
	rival->collidedTop = false;
	rival->collidedLeft = false;
	rival->collidedRight = false;
	for (GLuint i = 0; i < playerLasers.size(); i++) { delete playerLasers[i]; }
	playerLasers.clear();
}

void Alienated::ResetSingle(string level){
	for (GLuint i = 0; i < enemies.size(); i++) { delete enemies[i]; }
	enemies.clear();
	for (GLuint i = 0; i < deletedEnemies.size(); i++) { delete deletedEnemies[i]; }
	deletedEnemies.clear();
	for (GLuint i = 0; i < plasmaLocations.size(); i++) { delete plasmaLocations[i]; }
	plasmaLocations.clear();
	LoadTileMap(level);
	for (GLuint i = 0; i < enemies.size(); i++) { if (i % 2){ enemies[i]->flipped = !enemies[i]->flipped; enemies[i]->velocity_x = -enemies[i]->velocity_x; } }
}

float Alienated::timeUpdate(){
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
	
	return elapsed;
}

void Alienated::RenderBackground(){
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, background);
	GLfloat quad[] = { -2.66f, 2.00f, -2.66f, -2.00f, 2.66f, -2.00f, 2.66f, 2.00f };
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0 };
	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

bool Alienated::UpdateAndRender()
{	
	//draw background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	switch (state) {
		case STATE_MENU:
		{
			UpdateMenus();
			RenderBackground();
			RenderMenu();
		}
			break;
		case STATE_PVP:	
		{
			Update(timeUpdate());
			RenderBackground();
			Render();
		}
			break;
		case STATE_SINGLE_PLAYER:
		{
			Update(timeUpdate());
			RenderBackground();
			Render();
		}
			break;
		case STATE_GAME_OVER:
		{
			UpdateMenus();
			if (alive) RenderWin();
			else RenderLoss();
		}
			break;
		case STATE_GAME_PAUSED:
		{
			UpdateMenus();
			RenderPause();
		}
			break;
		case STATE_INSTRUCTIONS:
		{
			UpdateMenus();
			RenderBackground();
			RenderInstructions();
		}
			break;
		case STATE_ABOUT:
		{
			UpdateMenus();
			RenderBackground();
			RenderAbout();
		}
			break;
	}
	return done;
}

float lerp(float v0, float v1, float t) {
	return (1.0f - t)*v0 + t*v1;
}

int Alienated::worldToTileX(float worldX) {
	int x = (int)((worldX + (WORLD_OFFSET_X)) / TILE_SIZE);
	if (x<0 || x>mapWidth)return 0;
	return x;
}

int Alienated::worldToTileY(float worldY) {
	int y = (int)((-worldY + (WORLD_OFFSET_Y)) / TILE_SIZE);
	if (y<0 || y>mapHeight)return 0;
	return y;
}

void Alienated::collideWithMapX(GameObject * obj){
	if (obj->velocity_x < 0){
		if ((levelData[worldToTileY(obj->y - obj->height*OBJECT_SIZE*0.5f)][worldToTileX(obj->x - obj->width*OBJECT_SIZE)]) ||
			(levelData[worldToTileY(obj->y + obj->height*OBJECT_SIZE*0.5f)][worldToTileX(obj->x - obj->width*OBJECT_SIZE)]))
		{
			obj->collidedLeft = true;
			obj->x -= obj->velocity_x * FIXED_TIMESTEP;
			if (obj != player && obj != rival){//bounce for enemies
				obj->velocity_x = -obj->velocity_x; 
				obj->flipped = !obj->flipped;
			}
			else obj->velocity_x = 0.0f;//stop for player
			obj->acceleration_x = 0.0f;
		}
	}
	else if (obj->velocity_x > 0){
		if (((levelData[worldToTileY(obj->y - obj->height*OBJECT_SIZE*0.5f)][worldToTileX(obj->x + obj->width*OBJECT_SIZE)]) ||
			(levelData[worldToTileY(obj->y + obj->height*OBJECT_SIZE*0.5f)][worldToTileX(obj->x + obj->width*OBJECT_SIZE)])) &&
			((int)levelData[worldToTileY(obj->y)][worldToTileX(obj->x + obj->width*OBJECT_SIZE)] != 46 && (int)levelData[worldToTileY(obj->y)][worldToTileX(obj->x + obj->width*OBJECT_SIZE)] != 58))
		{
			obj->collidedRight = true;
			obj->x -= obj->velocity_x * FIXED_TIMESTEP;
			if (obj != player && obj != rival){//bounce for enemies
				obj->velocity_x = -obj->velocity_x;
				obj->flipped = !obj->flipped;
			}
			else obj->velocity_x = 0.0f;//stop for player
			obj->acceleration_x = 0.0f;
		}
	}
	else {
		obj->collidedLeft = false;
		obj->collidedRight = false;
	}
}

void Alienated::collideWithMapY(GameObject * obj){
	if (obj->velocity_y < 0){
		if ((levelData[worldToTileY(obj->y - obj->height*OBJECT_SIZE)][worldToTileX(obj->x + obj->width*OBJECT_SIZE*0.8f)]) ||
			(levelData[worldToTileY(obj->y - obj->height*OBJECT_SIZE)][worldToTileX(obj->x - obj->width*OBJECT_SIZE*0.8f)])){
			obj->collidedBottom = true;
			//obj->y -= obj->velocity_y * FIXED_TIMESTEP;
			obj->y -= fmod((obj->y - obj->height*OBJECT_SIZE), TILE_SIZE);
			if (obj->velocity_y < -0.5f&&obj->collidedBottom){ shakeValue = 0.02f; Mix_PlayChannel(-1, land, 0); }//shake screen
			obj->velocity_y = 0.0f; obj->acceleration_y = 0.0f;
		}
		//else obj->collidedBottom = false;
	}
	else if (obj->velocity_y > 0){
		if ((levelData[worldToTileY(obj->y + obj->height*OBJECT_SIZE)][worldToTileX(obj->x)]) /*||
			(levelData[worldToTileY(obj->y + obj->height*OBJECT_SIZE)][worldToTileX(obj->x - obj->width*OBJECT_SIZE*0.8f)])*/){
			obj->collidedTop = true;
			obj->y -= obj->velocity_y * FIXED_TIMESTEP;
			obj->velocity_y = 0.0f; obj->acceleration_y = 0.0f;
		}
		else obj->collidedTop = false;
	}
	else {
		//obj->collidedTop = false;
		if ((levelData[worldToTileY(obj->y - obj->height*OBJECT_SIZE*1.5f)][worldToTileX(obj->x + obj->width*OBJECT_SIZE*0.8f)]) ||
			(levelData[worldToTileY(obj->y - obj->height*OBJECT_SIZE*1.5f)][worldToTileX(obj->x - obj->width*OBJECT_SIZE*0.8f)])){
			obj->collidedBottom = true;
		}
		else obj->collidedBottom = false;
	}
}

void Alienated::moveAround(GameObject* obj){
	//Y movement
	obj->velocity_y = lerp(obj->velocity_y, 0.0f, FIXED_TIMESTEP * obj->friction_y);
	obj->velocity_y += obj->acceleration_y * FIXED_TIMESTEP;
	obj->y += obj->velocity_y * FIXED_TIMESTEP;

	if (!obj->collidedBottom){ obj->acceleration_y = -3.0f; }//gravity

	//collision detetection Y
	collideWithMapY(obj);

	//X movement
	obj->velocity_x = lerp(obj->velocity_x, 0.0f, FIXED_TIMESTEP * obj->friction_x);
	obj->velocity_x += obj->acceleration_x * FIXED_TIMESTEP;
	obj->x += obj->velocity_x * FIXED_TIMESTEP;

	//collision detection X
	collideWithMapX(obj);
}

void Alienated::moveAround2P(GameObject* first, GameObject* second){
	//Y movement
	first->velocity_y = lerp(first->velocity_y, 0.0f, FIXED_TIMESTEP * first->friction_y);
	first->velocity_y += first->acceleration_y * FIXED_TIMESTEP;
	first->y += first->velocity_y * FIXED_TIMESTEP;

	if (!first->collidedBottom){ first->acceleration_y = -3.0f; }//gravity

	collideWithMapY(first);

	second->velocity_y = lerp(second->velocity_y, 0.0f, FIXED_TIMESTEP * second->friction_y);
	second->velocity_y += second->acceleration_y * FIXED_TIMESTEP;
	second->y += second->velocity_y * FIXED_TIMESTEP;

	if (!second->collidedBottom){ second->acceleration_y = -3.0f; }//gravity

	collideWithMapY(second);

	if (first->collidesWithY(second) && second->collidesWithY(first)){
		first->y -= first->velocity_y * FIXED_TIMESTEP;
		first->velocity_y = 0.0f;
		second->y -= second->velocity_y * FIXED_TIMESTEP;
		second->velocity_y = 0.0f;
	}

	//X movement
	first->velocity_x = lerp(first->velocity_x, 0.0f, FIXED_TIMESTEP * first->friction_x);
	first->velocity_x += first->acceleration_x * FIXED_TIMESTEP;
	first->x += first->velocity_x * FIXED_TIMESTEP;

	collideWithMapX(first);

	second->velocity_x = lerp(second->velocity_x, 0.0f, FIXED_TIMESTEP * second->friction_x);
	second->velocity_x += second->acceleration_x * FIXED_TIMESTEP;
	second->x += second->velocity_x * FIXED_TIMESTEP;

	collideWithMapX(second);

	if (first->collidesWithX(second) && second->collidesWithX(first)){
		first->x -= first->velocity_x * FIXED_TIMESTEP;
		first->velocity_x = 0.0f;
		second->x -= second->velocity_x * FIXED_TIMESTEP;
		second->velocity_x = 0.0f;
	}
}

void destroyLaser(GameObject* l){
	l->rotation = 91.0f;
	l->velocity_x = 0.0f;
	if (l->v == 111.0f / 128.0f){
		l->u = 50.0f / 128.0f;
		l->v = 0.0f / 128.0f;
	}
	else{
		l->u = 0.0f / 128.0f;
		l->v = 48.0f / 128.0f;
	}
	l->w = 48.0f / 128.0f;
	l->h = 46.0f / 128.0f;
	l->width = 0.48f;
	l->height = 0.46f;
}

void Alienated::FixedUpdate(){
	shakeValue = lerp(shakeValue, 0.0f, FIXED_TIMESTEP);//gradually shake less
	if (shakeValue < 0.01f)shakeValue = 0.0f;//stop shaking after a while
	switch (state){
	case STATE_PVP:
	{
		moveAround(player);
		moveAround(rival);

		//plasma
		for (GLuint i = 0; i < plasmaLocations.size(); i++) {
			if (plasmaLocations[i]->flipped){
				if (plasmaLocations[i]->collidesWithY(player) || plasmaLocations[i]->collidesWithX(player)){
					player->bullets += 3;
					Mix_PlayChannel(-1, plasmaReload, 0);
					plasmaLocations[i]->flipped = false;
					GLuint next = rand() % plasmaLocations.size();
					while (next == i || plasmaLocations[next]->flipped)next = rand() % plasmaLocations.size();
					plasmaLocations[next]->flipped = true;
				}
				else if (plasmaLocations[i]->collidesWithY(rival) || plasmaLocations[i]->collidesWithX(rival)){
					rival->bullets += 3;
					Mix_PlayChannel(-1, plasmaReload, 0);
					plasmaLocations[i]->flipped = false;
					GLuint next = rand() % plasmaLocations.size();
					while (next == i || plasmaLocations[next]->flipped)next = rand() % plasmaLocations.size();
					plasmaLocations[next]->flipped = true;
				}
				else {
					plasmaLocations[i]->rotation += 360.0f*FIXED_TIMESTEP;
				}

			}

		}

		//collide lasers
		for (GLuint k = 0; k < playerLasers.size(); k++) {
			if (playerLasers[k]->rotation > 90.0f){
				playerLasers[k]->rotation += 2000.0f*FIXED_TIMESTEP;
			}
			else{
				playerLasers[k]->x += playerLasers[k]->velocity_x * FIXED_TIMESTEP;
				playerLasers[k]->y += playerLasers[k]->velocity_y * FIXED_TIMESTEP;
				if (playerLasers[k]->v == 96.0f / 128.0f && playerLasers[k]->collidesWithX(player)){
					player->hits += 1;
					destroyLaser(playerLasers[k]);
				}
				else if (playerLasers[k]->v == 111.0f / 128.0f && playerLasers[k]->collidesWithX(rival)){
					rival->hits += 1;
					destroyLaser(playerLasers[k]);
				}
				else if (levelData[worldToTileY(playerLasers[k]->y)][worldToTileX(playerLasers[k]->x)]){
					destroyLaser(playerLasers[k]);
				}

			}

		}
	}
		break;
	case STATE_SINGLE_PLAYER:
	{
		moveAround(player);
		if ((int)levelData[worldToTileY(player->y)][worldToTileX(player->x + player->width*OBJECT_SIZE)] == 46 || (int)levelData[worldToTileY(player->y)][worldToTileX(player->x + player->width*OBJECT_SIZE)] == 58){
			if (score == 1){ score = 2; Mix_PlayChannel(-1, winSound, 0); ResetSingle("level2.txt"); }//win level 1
			else if (score == 2){ score = 3; Mix_PlayChannel(-1, winSound, 0); ResetSingle("level3.txt"); }//win level 2
			else if (score == 3){ score = 4; Mix_PlayChannel(-1, winSound, 0); state = 5; }//win level 3
		}

		//collide enemies
		for (GLuint j = 0; j < enemies.size(); j++) {
			enemies[j]->velocity_x += enemies[j]->acceleration_x * FIXED_TIMESTEP;
			enemies[j]->x += enemies[j]->velocity_x * FIXED_TIMESTEP;

			collideWithMapX(enemies[j]);

			if (enemies[j]->collidesWithX(player)){//player died
				alive = false;
				state = 5;
				Mix_PlayChannel(-1, die, 0);
			}

			enemies[j]->velocity_y = lerp(enemies[j]->velocity_y, 0.0f, FIXED_TIMESTEP * enemies[j]->friction_y);
			enemies[j]->velocity_y += enemies[j]->acceleration_y * FIXED_TIMESTEP;
			enemies[j]->y += enemies[j]->velocity_y * FIXED_TIMESTEP;

			if (!enemies[j]->collidedBottom){ enemies[j]->acceleration_y = -2.0f; }//gravity

			collideWithMapY(enemies[j]);

			if (enemies[j]->collidesWithY(player)){//enemy destroyed
				//particleSource = new particleEmitter(enemies[j]->x, enemies[j]->y);
				enemies[j]->velocity_x = 0.0f;
				enemies[j]->width = 0.69f;  enemies[j]->height = 0.51f;  enemies[j]->u = 153.0f / 256; enemies[j]->w = 0.0f / 64; enemies[j]->w = 69.0f / 256; enemies[j]->h = 51.0f / 64;
				deletedEnemies.push_back(enemies[j]);
				enemies.erase(enemies.begin() + j);
				break;
			}
		}
	}
		break;
	}
}

void Alienated::animatePlayer(){
	bool walkAnimation = false;

	if (keys[SDL_SCANCODE_LEFT]) {//move left
		player->acceleration_x = -2.0f;
		player->flipped = true;
		walkAnimation = true;
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {//move right
		player->acceleration_x = 2.0f;
		player->flipped = false;
		walkAnimation = true;
	}
	else {
		player->acceleration_x = 0.0f;
		walkAnimation = false;
		walkAnimationTime = 0.8f;
	}

	if (abs(player->velocity_x) < 0.01f && abs(player->velocity_y) < 0.01f && !player->collidedTop){//p1_front
		player->u = 0.0f / 508.0f;
		player->v = 196.0f / 288.0f;
		player->w = 66.0f / 508.0f;
		player->h = 92.0f / 288.0f;
		player->width = 0.66f;
		player->height = 0.92f;
	}
	else if (player->velocity_y != 0.0f && !player->collidedBottom){//p1_jump
		player->u = 438.0f / 508.0f;
		player->v = 93.0f / 288.0f;
		player->w = 67.0f / 508.0f;
		player->h = 94.0f / 288.0f;
		player->width = 0.67f;
		player->height = 0.94f;
		walkAnimation = false;
	}
	else {//p1_stand
		player->u = 67.0f / 508.0f;
		player->v = 196.0f / 288.0f;
		player->w = 66.0f / 508.0f;
		player->h = 92.0f / 288.0f;
		player->width = 0.66f;
		player->height = 0.92f;
	}

	if (walkAnimation){
		player->w = 72.0f / 508.0f;
		player->h = 97.0f / 288.0f;
		player->width = 0.72f;
		player->height = 0.97f;
		float animationValue = fmod(walkAnimationTime, 1.1f);
		if (animationValue >= 0.0f && animationValue < 0.1f){ player->u = 0.0f / 508.0f; player->v = 0.0f / 288.0f; }
		else if (animationValue >= 0.1f && animationValue < 0.2f){ player->u = 73.0f / 508.0f; player->v = 0.0f / 288.0f; }
		else if (animationValue >= 0.2f && animationValue < 0.3f){ player->u = 146.0f / 508.0f; player->v = 0.0f / 288.0f; }
		else if (animationValue >= 0.3f && animationValue < 0.4f){ player->u = 0.0f / 508.0f; player->v = 98.0f / 288.0f; }
		else if (animationValue >= 0.4f && animationValue < 0.5f){ player->u = 73.0f / 508.0f; player->v = 98.0f / 288.0f; }
		else if (animationValue >= 0.5f && animationValue < 0.6f){ player->u = 146.0f / 508.0f; player->v = 98.0f / 288.0f; }
		else if (animationValue >= 0.6f && animationValue < 0.7f){ player->u = 219.0f / 508.0f; player->v = 0.0f / 288.0f; }
		else if (animationValue >= 0.7f && animationValue < 0.8f){ player->u = 292.0f / 508.0f; player->v = 0.0f / 288.0f; }
		else if (animationValue >= 0.8f && animationValue < 0.9f){ player->u = 219.0f / 508.0f; player->v = 98.0f / 288.0f; }
		else if (animationValue >= 0.9f && animationValue < 1.0f){ player->u = 365.0f / 508.0f; player->v = 0.0f / 288.0f; }
		else if (animationValue >= 1.0f && animationValue < 1.1f){ player->u = 292.0f / 508.0f; player->v = 98.0f / 288.0f; }
	}
}

void Alienated::animateRival(){
	bool walk2Animation = false;

	if (keys[SDL_SCANCODE_A]) {//move p2 left
		rival->acceleration_x = -2.0f;
		rival->flipped = true;
		walk2Animation = true;
	}
	else if (keys[SDL_SCANCODE_D]) {//move p2 right
		rival->acceleration_x = 2.0f;
		rival->flipped = false;
		walk2Animation = true;
	}
	else {
		rival->acceleration_x = 0.0f;
		walk2Animation = false;
		walk2AnimationTime = 0.8f;
	}

	if (abs(rival->velocity_x) < 0.01f && abs(rival->velocity_y) < 0.01f && !rival->collidedTop){//p2_front
		rival->u = 0.0f / 494.0f;
		rival->v = 190.0f / 282.0f;
		rival->w = 66.0f / 494.0f;
		rival->h = 92.0f / 282.0f;
		rival->width = 0.66f;
		rival->height = 0.92f;
	}
	else if (rival->velocity_y != 0.0f && !rival->collidedBottom){//p2_jump
		rival->u = 423.0f / 494.0f;
		rival->v = 95.0f / 282.0f;
		rival->w = 66.0f / 494.0f;
		rival->h = 94.0f / 282.0f;
		rival->width = 0.66f;
		rival->height = 0.94f;
		walk2Animation = false;
	}
	else {//p2_stand
		rival->u = 67.0f / 494.0f;
		rival->v = 190.0f / 282.0f;
		rival->w = 66.0f / 494.0f;
		rival->h = 92.0f / 282.0f;
		rival->width = 0.66f;
		rival->height = 0.92f;
	}

	if (walk2Animation){
		rival->w = 70.0f / 494.0f;
		rival->h = 94.0f / 282.0f;
		rival->width = 0.70f;
		rival->height = 0.94f;
		float animation2Value = fmod(walk2AnimationTime, 1.1f);
		if (animation2Value >= 0.0f && animation2Value < 0.1f){ rival->u = 0.0f / 494.0f; rival->v = 0.0f / 282.0f; }
		else if (animation2Value >= 0.1f && animation2Value < 0.2f){ rival->u = 71.0f / 494.0f; rival->v = 0.0f / 282.0f; }
		else if (animation2Value >= 0.2f && animation2Value < 0.3f){ rival->u = 142.0f / 494.0f; rival->v = 0.0f / 282.0f; }
		else if (animation2Value >= 0.3f && animation2Value < 0.4f){ rival->u = 0.0f / 494.0f; rival->v = 95.0f / 282.0f; }
		else if (animation2Value >= 0.4f && animation2Value < 0.5f){ rival->u = 71.0f / 494.0f; rival->v = 95.0f / 282.0f; }
		else if (animation2Value >= 0.5f && animation2Value < 0.6f){ rival->u = 142.0f / 494.0f; rival->v = 95.0f / 282.0f; }
		else if (animation2Value >= 0.6f && animation2Value < 0.7f){ rival->u = 213.0f / 494.0f; rival->v = 0.0f / 282.0f; }
		else if (animation2Value >= 0.7f && animation2Value < 0.8f){ rival->u = 284.0f / 494.0f; rival->v = 0.0f / 282.0f; }
		else if (animation2Value >= 0.8f && animation2Value < 0.9f){ rival->u = 213.0f / 494.0f; rival->v = 95.0f / 282.0f; }
		else if (animation2Value >= 0.9f && animation2Value < 1.0f){ rival->u = 355.0f / 494.0f; rival->v = 0.0f / 282.0f; }
		else if (animation2Value >= 1.0f && animation2Value < 1.1f){ rival->u = 284.0f / 494.0f; rival->v = 95.0f / 282.0f; }
	}
}

void Alienated::animateEnemies(){
	float animationValue = fmod(timePassed, 0.2f);
	for (GLuint j = 0; j < enemies.size(); j++) {
		if (animationValue >= 0.0f && animationValue < 0.1f){ enemies[j]->width = 0.72f;  enemies[j]->height = 0.51f;  enemies[j]->u = 79.0f / 256; enemies[j]->w = 0.0f / 64; enemies[j]->w = 72.0f / 256; enemies[j]->h = 51.0f / 64; }
		else if (animationValue >= 0.1f && animationValue < 0.2f){ enemies[j]->width = 0.77f;  enemies[j]->height = 0.53f;  enemies[j]->u = 0.0f / 256; enemies[j]->w = 0.0f / 64; enemies[j]->w = 77.0f / 256; enemies[j]->h = 53.0f / 64; }
	}
}

void Alienated::drawLives(){
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, rivalHUD);
	GLfloat quad[] = { -0.08f, 0.08f, -0.08f, -0.08f, 0.08f, -0.08f, 0.08f, 0.08f };
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0 };
	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTranslatef(-2.5f, 1.9f, 0.0);
	for (GLuint i = rival->hits; i < MAX_HITS; i++){//display lives
		glDrawArrays(GL_QUADS, 0, 4);
		glTranslatef(0.22f, 0.0f, 0.0);
	}
	glBindTexture(GL_TEXTURE_2D, playerHUD);
	glLoadIdentity();
	glTranslatef(2.5f, 1.9f, 0.0);
	for (GLuint i = player->hits; i < MAX_HITS; i++){//display lives
		glDrawArrays(GL_QUADS, 0, 4);
		glTranslatef(-0.22f, 0.0f, 0.0);
	}
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void Alienated::Update(float elapsed)
{
	timePassed += elapsed;
	if (timePassed > 100000000000000000000.0f)timePassed -= 100000000000000000.0f;
	
	walkAnimationTime += elapsed;
	animatePlayer();

	switch (state){
		case STATE_PVP:
		{
			if (rival->hits >= MAX_HITS){
				state = 5; playerScore++; 
				Mix_PlayChannel(-1, die, 0);
				return;
			}
			else if (player->hits >= MAX_HITS){
				state = 5; rivalScore++; 
				Mix_PlayChannel(-1, die, 0);
				return;
			}
			walk2AnimationTime += elapsed;
			animateRival();

			//animate and remove lasers
			for (GLuint k = 0; k < playerLasers.size(); k++) {
				if (playerLasers[k]->rotation > 90.0f){
					if (playerLasers[k]->rotation > 360.0f){
						delete playerLasers[k];
						playerLasers.erase(playerLasers.begin() + k);
						break;
					}
					else if (playerLasers[k]->rotation > 270.0f){
						if (playerLasers[k]->u == 0.0f){
							playerLasers[k]->v = 0.0f / 128.0f;
						}
						else{
							playerLasers[k]->v = 48.0f / 128.0f;
						}
						playerLasers[k]->w = 48.0f / 128.0f;
						playerLasers[k]->h = 46.0f / 128.0f;
					}
				}
			}
		}
			break;
		case STATE_SINGLE_PLAYER:
		{	
			animateEnemies();
		}
			break;
	}

	//if (keys[SDL_SCANCODE_UP] && player->collidedBottom){//jump
	//	player->velocity_y = 3.0f;
	//	player->collidedBottom = false;
	//	Mix_PlayChannel(-1, jumpSound, 0);
	//}
	//if (keys[SDL_SCANCODE_DOWN] /*&& player->collidedBottom*/){//duck
	//	player->u = 365.0f / 508.0f;
	//	player->v = 98.0f / 288.0f;
	//	player->w = 69.0f / 508.0f;
	//	player->h = 71.0f / 288.0f;
	//	player->width = 0.69f;
	//	player->height = 0.71f;
	//}


	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_M){
				if (Mix_PausedMusic() == 1)//music paused
				{
					Mix_ResumeMusic();
				}
				//music is playing
				else
				{
					Mix_PauseMusic();
				}
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_P) {
				state = 4;
			}

			switch (state){
			case STATE_PVP:
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_RCTRL) {
					if (player->bullets){
						player->bullets -= 1;
						Mix_PlayChannel(-1, laser, 0);
						if (keys[SDL_SCANCODE_UP]){
							playerLasers.push_back(new GameObject(laserSheet, player->x, player->y + 0.1f, 0.37f, 0.13f, 0.0f, 5.0f, 90.0f, 0.0f / 128.0f, 111.0f / 128.0f, 37.0f / 128.0f, 13.0f / 128.0f, 0, false));
						}
						else{
							playerLasers.push_back(new GameObject(laserSheet, player->x, player->y + 0.01f, 0.37f, 0.13f, 5.0f, 0.0f, 0.0f, 0.0f / 128.0f, 111.0f / 128.0f, 37.0f / 128.0f, 13.0f / 128.0f, 0, false));
							if (player->flipped)playerLasers[playerLasers.size() - 1]->velocity_x = -playerLasers[playerLasers.size() - 1]->velocity_x;
						}
					}
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_UP && player->collidedBottom){
					player->velocity_y = 3.0f;
					player->collidedBottom = false;
					Mix_PlayChannel(-1, jumpSound, 0);
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_LCTRL) {
					if (rival->bullets){
						rival->bullets -= 1;
						Mix_PlayChannel(-1, laser, 0);
						if (keys[SDL_SCANCODE_W]){
							playerLasers.push_back(new GameObject(laserSheet, rival->x, rival->y + 0.01f, 0.37f, 0.13f, 0.0f, 5.0f, 90.0f, 0.0f / 128.0f, 96.0f / 128.0f, 37.0f / 128.0f, 13.0f / 128.0f, 0, false));
						}
						else{
							playerLasers.push_back(new GameObject(laserSheet, rival->x, rival->y + 0.01f, 0.37f, 0.13f, 5.0f, 0.0f, 0.0f, 0.0f / 128.0f, 96.0f / 128.0f, 37.0f / 128.0f, 13.0f / 128.0f, 0, false));
							if (rival->flipped)playerLasers[playerLasers.size() - 1]->velocity_x = -playerLasers[playerLasers.size() - 1]->velocity_x;
						}
					}
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_W && rival->collidedBottom){
					rival->velocity_y = 3.0f;
					rival->collidedBottom = false;
					Mix_PlayChannel(-1, jumpSound, 0);
				}
			}
				break;
			case STATE_SINGLE_PLAYER:
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_UP && player->collidedBottom){
					player->velocity_y = 3.0f;
					player->collidedBottom = false;
					Mix_PlayChannel(-1, jumpSound, 0);
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_1){
					score = 1; ResetSingle("level1.txt");
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_2){
					score = 2; ResetSingle("level2.txt");
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_3){
					score = 3; ResetSingle("level3.txt");
				}
			}
				break;
			}
			
		}
	}
}

void Alienated::Render()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	switch (state){
	case STATE_PVP:
	{
		glTranslatef(-TILE_SIZE * mapWidth / 2, TILE_SIZE * mapHeight / 2, 0.0f);
		glTranslatef(0.0f, sin(timePassed * 50)*shakeValue, 0.0f);//shake screen
		renderTileMap();
		player->DrawSprite(OBJECT_SIZE);
		rival->DrawSprite(OBJECT_SIZE);
		drawLives();
		for (GLuint i = 0; i < playerLasers.size(); i++) { playerLasers[i]->DrawSprite(OBJECT_SIZE); }
		for (GLuint i = 0; i < plasmaLocations.size(); i++) { if (plasmaLocations[i]->flipped)plasmaLocations[i]->DrawSprite(OBJECT_SIZE); }
		glLoadIdentity();
		glTranslatef(-2.4f, 1.7f, 0.0);
		PrintText(fontTexture, to_string(rival->bullets), 0.2f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f);
		glLoadIdentity();
		glTranslatef(2.2f, 1.7f, 0.0);
		if (player->bullets <= 9)glTranslatef(0.2f, 0.0f, 0.0);
		PrintText(fontTexture, to_string(player->bullets), 0.2f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f);
	}
		break;
	case STATE_SINGLE_PLAYER:
	{
		if (player->x>2.66f&&player->x<10.1f&&score<3)glTranslatef(-player->x, TILE_SIZE * mapHeight / 2, 0.0f);
		else if (player->x<2.66f)glTranslatef(-2.66f, TILE_SIZE * mapHeight / 2, 0.0f);
		else if (score<3)glTranslatef(-10.1f, TILE_SIZE * mapHeight / 2, 0.0f);
		else {
			if (player->x > 14.47f)glTranslatef(-14.47f, TILE_SIZE * mapHeight / 2, 0.0f);
			else glTranslatef(-player->x, TILE_SIZE * mapHeight / 2, 0.0f);
		}
		glTranslatef(0.0f, sin(timePassed * 50)*shakeValue, 0.0f);//shake screen
		renderTileMap();
		for (GLuint i = 0; i < deletedEnemies.size(); i++) { deletedEnemies[i]->DrawSprite(OBJECT_SIZE); }
		player->DrawSprite(OBJECT_SIZE);
		for (GLuint i = 0; i < enemies.size(); i++) { enemies[i]->DrawSprite(OBJECT_SIZE); }
	}
		break;
	}
	
	glPopMatrix();
	SDL_GL_SwapWindow(displayWindow);
}

void Alienated::renderTileMap()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, spriteSheet);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	vector<float> vertexData;
	vector<float> texCoordData;
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			if (levelData[y][x]) {
				float u = (float)(((int)levelData[y][x] - 1) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)levelData[y][x] - 1) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
				float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
				float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
				vertexData.insert(vertexData.end(), {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
				});
				texCoordData.insert(texCoordData.end(), { u, v,
					u, v + (spriteHeight),
					u + spriteWidth, v + (spriteHeight),
					u + spriteWidth, v
				});
			}
		}
	}
	glVertexPointer(2, GL_FLOAT, 0, vertexData.data());
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoordData.data());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_QUADS, 0, vertexData.size() / 2);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void Alienated::UpdateMenus(){
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN){
			if (event.key.keysym.scancode == SDL_SCANCODE_M){
				if (Mix_PausedMusic() == 1)//music paused
				{
					Mix_ResumeMusic();
				}
				//music is playing
				else
				{
					Mix_PauseMusic();
				}
			}
			switch (state) {
			case STATE_MENU:
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE || event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
					Mix_PlayChannel(-1, select, 0);
					if (selection == 1)state = 2;
					else if (selection == 2)state = 6;
					else if (selection == 3)state = 7;
					else if (selection == 0){ state = 3; ResetSingle("level1.txt"); score = 1; }
					else if (selection == 4)done = true;

				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_UP){
					Mix_PlayChannel(-1, menu, 0);
					if (selection == 0)selection = 4;
					else selection -= 1;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN){
					Mix_PlayChannel(-1, menu, 0);
					if (selection == 4)selection = 0;
					else selection += 1;
				}
				
			}
				break;
			case STATE_GAME_OVER:
			{	
				if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
					ResetGame();
					playerScore = 0;
					rivalScore = 0;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					if (score){ 
							if (score==1){
								ResetGame(); 
								score = 1; 
								ResetSingle("level1.txt");
								state = 3; 
							}
							else if (score == 2){
								ResetGame();
								score = 2;
								ResetSingle("level2.txt");
								state = 3;
							}
							else if (score == 3){
								ResetGame();
								score = 3;
								ResetSingle("level3.txt");
								state = 3;
							}
					}
					else {
						ResetGame(); 
						state = 2;
						//Switch locations
						if ((playerScore+rivalScore)%2){
							float x = player->x;
							float y = player->y;
							player->x = rival->x;
							player->y = rival->y;
							rival->x = x;
							rival->y = y;
						}
					}
				}
			}
				break;
			case STATE_GAME_PAUSED:
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_P || event.key.keysym.scancode == SDL_SCANCODE_SPACE || event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
					if (score)state = 3;
					else state = 2;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_R) {
					ResetGame();
				}
			}
				break;
			case STATE_INSTRUCTIONS:
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE || event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
					state = 1;
				}
			}
				break;
			case STATE_ABOUT:
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE || event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
					state = 1;
				}
			}
				break;
			}
		}
	}
}

void Alienated::RenderMenu(){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.0f, 1.0f, 0.0f);
	/*PrintText(fontTexture, "ALIENATED", 0.3f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);*/
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, title);
	GLfloat quad[] = { -2.66f, 2.66f * 150 / 800, -2.66f, -2.66f * 150 / 800, 2.66f, -2.66f * 150 / 800, 2.66f, 2.66f * 150 / 800 };
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0 };
	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glLoadIdentity();
	glTranslatef(-0.9f, 0.0f, 0.0);
	if (selection == 0)PrintText(fontTexture, "Single Player", 0.2f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f);
	else PrintText(fontTexture, "Single Player", 0.15f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(0.0f, -0.3f, 0.0);
	if(selection == 1)PrintText(fontTexture, "PVP Arena", 0.2f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f);
	else PrintText(fontTexture, "PVP Arena", 0.15f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(0.0f, -0.3f, 0.0);
	if (selection == 2)PrintText(fontTexture, "Instructions", 0.2f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f);
	else PrintText(fontTexture, "Instructions", 0.15f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(0.0f, -0.3f, 0.0);
	if (selection == 3)PrintText(fontTexture, "About", 0.2f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f);
	else PrintText(fontTexture, "About", 0.15f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(0.0f, -0.3f, 0.0);
	if (selection == 4)PrintText(fontTexture, "Quit", 0.2f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f);
	else PrintText(fontTexture, "Quit", 0.15f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	SDL_GL_SwapWindow(displayWindow);
	glPopMatrix();
}

void Alienated::RenderWin(){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	if (rival->hits >= MAX_HITS){
		glTranslatef(-1.7f, 0.5f, 0.0);
		PrintText(fontTexture, "PLAYER WINS!", 0.3f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f);
		glTranslatef(0.5f, -0.5f, 0.0);
		PrintText(fontTexture, "Player:" + to_string(playerScore) + " Rival:" + to_string(rivalScore), 0.15f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glTranslatef(-0.4f, -0.5f, 0.0);
		PrintText(fontTexture, "SPACE to continue, ENTER to quit", 0.1f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	else if (player->hits >= MAX_HITS){
		glTranslatef(-1.5, 0.5f, 0.0);
		PrintText(fontTexture, "RIVAL WINS!", 0.3f, 0.0f, 0.0f, 0.0f, 0.7f, 1.0f);
		glTranslatef(0.4f, -0.5f, 0.0);
		PrintText(fontTexture, "Rival:" + to_string(rivalScore) + " Player:" + to_string(playerScore), 0.15f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		glTranslatef(-0.4f, -0.5f, 0.0);
		PrintText(fontTexture, "SPACE to continue, ENTER to quit", 0.1f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	else{
		glTranslatef(-1.4f, 0.5f, 0.0);
		PrintText(fontTexture, "YOU WIN!", 0.4f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		glTranslatef(-0.1f, -0.5f, 0.0);
		PrintText(fontTexture, "SPACE to restart, ENTER to quit", 0.1f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	SDL_GL_SwapWindow(displayWindow);
	glPopMatrix();
}

void Alienated::RenderLoss(){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(-1.5f, 0.5f, 0.0);
	PrintText(fontTexture, "YOU DIED!", 0.4f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.5f, 0.0);
	PrintText(fontTexture, "SPACE to restart, ENTER to quit", 0.1f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	SDL_GL_SwapWindow(displayWindow);
	glPopMatrix();
}

void Alienated::RenderPause(){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(-1.5f, 0.0f, 0.0);
	PrintText(fontTexture, "GAME PAUSED", 0.3f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	glTranslatef(0.2f, -0.5f, 0.0);
	PrintText(fontTexture, "P to continue, R to return", 0.1f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	SDL_GL_SwapWindow(displayWindow);
	glPopMatrix();
}

void Alienated::RenderInstructions(){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(-1.72f, 1.7f, 0.0);
	PrintText(fontTexture, "Instructions", 0.3f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glLoadIdentity();
	glTranslatef(-2.0f, 1.2f, 0.0);
	PrintText(fontTexture, "PLAYER:", 0.25f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f);
	glTranslatef(0.5f, -0.5f, 0.0);
	PrintText(fontTexture, "ARROWS to Move", 0.2f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(0.0f, -0.5f, 0.0);
	PrintText(fontTexture, "Right CTRL to Shoot", 0.2f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(-0.5f, -0.5f, 0.0);
	PrintText(fontTexture, "RIVAL:", 0.25f, 0.0f, 0.0f, 0.0f, 0.7f, 1.0f);
	glTranslatef(0.5f, -0.5f, 0.0);
	PrintText(fontTexture, "W, A, D to Move", 0.2f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(0.0f, -0.5f, 0.0);
	PrintText(fontTexture, "Left CTRL to Shoot", 0.2f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(-0.9f, -0.4f, 0.0);
	PrintText(fontTexture, "P to Pause Game, M to pause music", 0.15f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	SDL_GL_SwapWindow(displayWindow);
	glPopMatrix();
}

void Alienated::RenderAbout(){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(-1.1f, 1.5f, 0.0);
	PrintText(fontTexture, "NYU-Poly", 0.3f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(0.3f, -0.4f, 0.0);
	PrintText(fontTexture, "CS3113", 0.3f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(-1.05f, -0.4f, 0.0);
	PrintText(fontTexture, "Final Project", 0.3f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(-0.3f, -1.0f, 0.0);
	PrintText(fontTexture, "Roberts Plaudis", 0.3f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(0.9f, -0.4f, 0.0);
	PrintText(fontTexture, "N15592461", 0.3f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(-0.7f, -0.7f, 0.0);
	PrintText(fontTexture, "Music: Dum Dee Dum", 0.2f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	glTranslatef(0.7f, -0.4f, 0.0);
	PrintText(fontTexture, "	by Keys n Krates", 0.2f, 0.0f, 0.75f, 0.75f, 1.0f, 1.0f);
	SDL_GL_SwapWindow(displayWindow);
	glPopMatrix();
}