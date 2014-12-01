#include "Alienated.h"

#define FIXED_TIMESTEP 0.0166666f // 60 FPS (1.0f/60.0f)
#define MAX_TIMESTEPS 6
#define SPRITE_COUNT_X 12
#define SPRITE_COUNT_Y 13
#define TILE_SIZE 0.2f
#define OBJECT_SIZE 0.2f
#define WORLD_OFFSET_X 0
#define WORLD_OFFSET_Y 0

using namespace std;

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	if (!surface){ return -1; }
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	/**/
	if (image_path == "tiles_spritesheet.png"){
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
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

Alienated::Alienated() :keys(SDL_GetKeyboardState(NULL))
{
	Init();
	LoadTileMap();
	ResetGame();
}

Alienated::~Alienated()
{
	delete player;
	for (GLuint i = 0; i < enemies.size(); i++) { delete enemies[i]; }
	Mix_FreeChunk(jumpSound);
	Mix_FreeChunk(winSound);
	Mix_FreeMusic(music);
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
	//glOrtho(-1.33 * 10, 1.33 * 10, -1.0 * 10, 1.0 * 10, -1.0, 1.0);
	//glOrtho(-1.0 , 1.0 , -1.0 , 1.0 , -1.0, 1.0);
	glOrtho(-1.33 * 2, 1.33 * 2, -1.0 * 2, 1.0 * 2, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	fontTexture = LoadTexture("pixel_font.png");
	spriteSheet = LoadTexture("tiles_spritesheet.png");
	jumpSound = Mix_LoadWAV("jumpSound.wav");
	winSound = Mix_LoadWAV("triumph.wav");
	music = Mix_LoadMUS("music.mp3");
	Mix_PlayMusic(music, -1);
	GLuint playerSpriteSheet = LoadTexture("p1_spritesheet.png");
	player = new GameObject(playerSpriteSheet, 0.0f, 0.0f, 0.66f, 0.92f, 0.0f, 0.0f, 0.0f, 0.0f / 508.0f, 196.0f / 288.0f, 66.0f / 508.0f, 92.0f / 288.0f, 1.0f, false, true);
}

void Alienated::LoadTileMap(){
	//ifstream infile("level.txt");
	ifstream infile("arena.txt");
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
		else if (line == "[ObjectsLayer]") {
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
			float placeX = atoi(xPosition.c_str()) / 16 * TILE_SIZE;
			float placeY = atoi(yPosition.c_str()) / 16 * -TILE_SIZE;
			//placeEntity(type, placeX, placeY);
			enemies.push_back(new GameObject(spriteSheet, placeX, placeY, 0.75f, 0.75f, 1.5f, 0.0f, 0.0f, //width,height,dx,dy,rot
				(float)(80 % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X, //u
				(float)((80) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y, //v
				1.0f / (float)SPRITE_COUNT_X, //w
				1.0f / (float)SPRITE_COUNT_Y, //h
				1.0f, false, true));
		}
	}
	return true;
}

void Alienated::ResetGame(){
	score = 0;
	alive = true;
	done = false;
	lastFrameTicks = 0.0f;
	timeLeftOver = 0.0f;
	timePassed = 0.0f;
	walkAnimationTime = 0.0f;
	player->x = 0.5f;
	player->y = -0.5;
	player->velocity_x = 0.0f;
	player->velocity_y = 0.0f;
	player->acceleration_x = 0.0f;
	player->acceleration_y = 0.0f;
	player->collidedBottom = false;
	player->collidedTop = false;
	player->collidedLeft = false;
	player->collidedRight = false;
	for (GLuint i = 0; i < deletedEnemies.size(); i++) { enemies.push_back(deletedEnemies[i]); }
	deletedEnemies.clear();
}

bool Alienated::UpdateAndRender()
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
		if ((levelData[worldToTileY(obj->y - obj->height*OBJECT_SIZE*0.9f)][worldToTileX(obj->x - obj->width*OBJECT_SIZE)]) ||
			(levelData[worldToTileY(obj->y + obj->height*OBJECT_SIZE*0.9f)][worldToTileX(obj->x - obj->width*OBJECT_SIZE)]))
		{
			obj->collidedLeft = true;
			obj->x -= obj->velocity_x * FIXED_TIMESTEP;
			if (obj->x == player->x&& obj->y == player->y)obj->velocity_x = 0.0f;//stop for player
			else obj->velocity_x = -obj->velocity_x;//bounce for enemies
			obj->acceleration_x = 0.0f;
		}
	}
	else if (obj->velocity_x > 0){
		if ((levelData[worldToTileY(obj->y - obj->height*OBJECT_SIZE*0.9f)][worldToTileX(obj->x + obj->width*OBJECT_SIZE)]) ||
			(levelData[worldToTileY(obj->y + obj->height*OBJECT_SIZE*0.9f)][worldToTileX(obj->x + obj->width*OBJECT_SIZE)]))
		{
			obj->collidedRight = true;
			obj->x -= obj->velocity_x * FIXED_TIMESTEP;
			if (obj->x == player->x&& obj->y == player->y)obj->velocity_x = 0.0f;//stop for player
			else obj->velocity_x = -obj->velocity_x;//bounce for enemies
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
			if (obj->velocity_y < -0.5f&&obj->collidedBottom){ shakeValue = 0.02f; }//shake screen
			obj->velocity_y = 0.0f; obj->acceleration_y = 0.0f;
		}
		//else obj->collidedBottom = false;
	}
	else if (obj->velocity_y > 0){
		if ((levelData[worldToTileY(obj->y + obj->height*OBJECT_SIZE)][worldToTileX(obj->x + obj->width*OBJECT_SIZE*0.8f)]) ||
			(levelData[worldToTileY(obj->y + obj->height*OBJECT_SIZE)][worldToTileX(obj->x - obj->width*OBJECT_SIZE*0.8f)])){
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

void Alienated::FixedUpdate(){
	if (worldToTileX(player->x) >= 120 && score == 0){ score = 1; Mix_PlayChannel(-1, winSound, 0); }//win

	shakeValue = lerp(shakeValue, 0.0f, FIXED_TIMESTEP);//gradually shake less
	if (shakeValue < 0.01f)shakeValue = 0.0f;//stop shaking after a while

	//Y movement
	player->velocity_y = lerp(player->velocity_y, 0.0f, FIXED_TIMESTEP * player->friction_y);
	player->velocity_y += player->acceleration_y * FIXED_TIMESTEP;
	player->y += player->velocity_y * FIXED_TIMESTEP;

	if (!player->collidedBottom){ player->acceleration_y = -3.0f; }//gravity

	//collision detetection Y
	collideWithMapY(player);

	//X movement
	player->velocity_x = lerp(player->velocity_x, 0.0f, FIXED_TIMESTEP * player->friction_x);
	player->velocity_x += player->acceleration_x * FIXED_TIMESTEP;
	player->x += player->velocity_x * FIXED_TIMESTEP;

	//collision detection X
	collideWithMapX(player);

	for (GLuint j = 0; j < enemies.size(); j++) {
		enemies[j]->velocity_x += enemies[j]->acceleration_x * FIXED_TIMESTEP;
		enemies[j]->x += enemies[j]->velocity_x * FIXED_TIMESTEP;

		collideWithMapX(enemies[j]);

		if (enemies[j]->collidesWithX(player)){
			ResetGame();
			break;
		}

		enemies[j]->velocity_y = lerp(enemies[j]->velocity_y, 0.0f, FIXED_TIMESTEP * enemies[j]->friction_y);
		enemies[j]->velocity_y += enemies[j]->acceleration_y * FIXED_TIMESTEP;
		enemies[j]->y += enemies[j]->velocity_y * FIXED_TIMESTEP;

		if (!enemies[j]->collidedBottom){ enemies[j]->acceleration_y = -2.0f; }//gravity

		collideWithMapY(enemies[j]);

		if (enemies[j]->collidesWithY(player)){
			//particleSource = new particleEmitter(enemies[j]->x, enemies[j]->y);
			deletedEnemies.push_back(enemies[j]);
			enemies.erase(enemies.begin() + j);
			break;
		}
	}

}

void Alienated::Update(float elapsed)
{
	//if (particleSource)particleSource->Update(elapsed);
	timePassed += elapsed;
	walkAnimationTime += elapsed;
	bool walkAnimation = false;
	//if (timePassed > 100000000000000000000.0f)timePassed -= 100000000000000000000.0f;
	if (keys[SDL_SCANCODE_LEFT]) {//move left
		player->acceleration_x = -1.0f;
		player->flipped = true;
		walkAnimation = true;
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {//move right
		player->acceleration_x = 1.0f;
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
		if (animationValue >= 0.0f && animationValue < 0.1f){player->u = 0.0f / 508.0f;player->v = 0.0f / 288.0f;}
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

	if (keys[SDL_SCANCODE_UP] && player->collidedBottom){//jump
		player->velocity_y = 3.0f;
		player->collidedBottom = false;
		Mix_PlayChannel(-1, jumpSound, 0);
	}
	else if (keys[SDL_SCANCODE_DOWN] /*&& player->collidedBottom*/){//duck
		player->u = 365.0f / 508.0f;
		player->v = 98.0f / 288.0f;
		player->w = 69.0f / 508.0f;
		player->h = 71.0f / 288.0f;
		player->width = 0.69f;
		player->height = 0.71f;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
}

void Alienated::Render()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	if (score){ glTranslatef(-0.8f, 0.0f, 0.0f); PrintText(fontTexture, "YOU WIN!", 0.25f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f); }//win

	else{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		//if (player->y>-4.4 && player->x>2 * 1.33)
		//	glTranslatef(-player->x, -player->y, 0.0f); //scrolling
		//else{//boundary lock
		//	float y = player->y;
		//	float x = player->x;
		//	if (y < -4.4f)y = -4.4f;
		//	if (x < 2 * 1.00f)x = 2 * 1.00f;
		//	glTranslatef(-x, -y, 0.0f);
		//}
		//glTranslatef(noise1(perlinValue), noise1(perlinValue + 10.0), 0.0);
		glTranslatef(-TILE_SIZE * mapWidth / 2, TILE_SIZE * mapHeight/ 2, 0.0f);
		glTranslatef(0.0f, sin(timePassed * 50)*shakeValue, 0.0f);//shake screen
		renderTileMap();
		for (GLuint i = 0; i < enemies.size(); i++) { enemies[i]->DrawSprite(OBJECT_SIZE); }
		player->DrawSprite(OBJECT_SIZE);
		//if (particleSource) particleSource->Render();
		glPopMatrix();
	}
	SDL_GL_SwapWindow(displayWindow);
}

void Alienated::renderTileMap()
{
	//glLoadIdentity();
	//glTranslatef(-TILE_SIZE * mapWidth / 2, TILE_SIZE * mapHeight / 2, 0.0f);
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
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	glDrawArrays(GL_QUADS, 0, vertexData.size() / 2);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}