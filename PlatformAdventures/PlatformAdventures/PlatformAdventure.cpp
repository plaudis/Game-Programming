#include "PlatformAdventure.h"

#define FIXED_TIMESTEP 0.0166666f // 60 FPS (1.0f/60.0f)
#define MAX_TIMESTEPS 6
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8
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


PlatformAdventure::PlatformAdventure() :keys(SDL_GetKeyboardState(NULL))
{
	Init();
	LoadMap();
	ResetGame();
}


PlatformAdventure::~PlatformAdventure()
{
	delete player;
	for (GLuint i = 0; i < enemies.size(); i++) { delete enemies[i]; }
	SDL_Quit();
}

void PlatformAdventure::Init()
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("PlatformAdventure", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

	glViewport(0, 0, 800, 600);
	glMatrixMode(GL_PROJECTION);
	//glOrtho(-1.33 * 10, 1.33 * 10, -1.0 * 10, 1.0 * 10, -1.0, 1.0);
	//glOrtho(-1.33 , 1.33 , -1.0 , 1.0 , -1.0, 1.0);
	glOrtho(-1.33 * 2, 1.33 * 2, -1.0 * 2, 1.0 * 2, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	fontTexture = LoadTexture("pixel_font.png");
	spriteSheet = LoadTexture("arne_sprites.png");
}

void PlatformAdventure::LoadMap(){
	GLuint playerSprite = LoadTexture("p1_front.png");
	player = new GameObject(playerSprite, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, false, true);

	ifstream infile("level.txt");
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

bool PlatformAdventure::readHeader(std::ifstream &stream) {
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

bool PlatformAdventure::readLayerData(std::ifstream &stream) {
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
						levelData[y][x] = val - 1;
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

bool PlatformAdventure::readEntityData(std::ifstream &stream) {
	string line;
	string type;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "Enemy") {
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
			//if (type == "Enemy"){ enemies.push_back(new GameObject(spriteSheet, placeX, placeY, 0.0f, 0.0f, 0.0f, 196.0f / 256.0f, 94.0f / 256.0f, 50.0f / 256.0f, 28.0f / 256.0f, 1.0f, false, true)); }
		}
	}
	return true;
}

void PlatformAdventure::ResetGame(){
	score = 0;
	alive = true;
	done = false;
	lastFrameTicks = 0.0f;
	timeLeftOver = 0.0f;
	timePassed = 0.0f;
	player->x = 0.5f;
	player->y = -1.0f;
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

bool PlatformAdventure::UpdateAndRender()
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

//void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
//	*gridX = (int)((worldX + (WORLD_OFFSET_X)) / TILE_SIZE);
//	*gridY = (int)((-worldY + (WORLD_OFFSET_Y)) / TILE_SIZE);
//}

int PlatformAdventure::worldToTileX(float worldX) {
	int x =(int)((worldX + (WORLD_OFFSET_X)) / TILE_SIZE);
	if (x<0 || x>mapWidth)return 0;
	return x;
}

int PlatformAdventure::worldToTileY(float worldY) {
	int y = (int)((-worldY + (WORLD_OFFSET_Y)) / TILE_SIZE);
	if (y<0 || y>mapHeight)return 0;
	return y;
}

void PlatformAdventure::FixedUpdate(){
	if (player->x + player->width*0.2f > 1.2f && player->y>0.35f){score = 1;}//win

	//Y movement
	player->velocity_y = lerp(player->velocity_y, 0.0f, FIXED_TIMESTEP * player->friction_y);
	player->velocity_y += player->acceleration_y * FIXED_TIMESTEP;
	player->y += player->velocity_y * FIXED_TIMESTEP;

	if (!player->collidedBottom){ player->acceleration_y = -2.0f; }//gravity

	//collision detetection Y
	if (player->velocity_y < 0){
		if (levelData[worldToTileY(player->y - player->height*OBJECT_SIZE)][worldToTileX(player->x)]){
			player->collidedBottom = true;
			player->y -= player->velocity_y * FIXED_TIMESTEP;
			player->velocity_y = 0.0f; player->acceleration_y = 0.0f;
		}
	}
	else if (player->velocity_y > 0){
		if (levelData[worldToTileY(player->y + player->height*OBJECT_SIZE)][worldToTileX(player->x)]){
			player->collidedTop = true;
			player->y -= player->velocity_y * FIXED_TIMESTEP;
			player->velocity_y = 0.0f; player->acceleration_y = 0.0f;
		}
	}
	else player->collidedTop = false;


	//X movement
	player->velocity_x = lerp(player->velocity_x, 0.0f, FIXED_TIMESTEP * player->friction_x);
	player->velocity_x += player->acceleration_x * FIXED_TIMESTEP;
	player->x += player->velocity_x * FIXED_TIMESTEP;

	//collision detection X
	if (player->velocity_x < 0){
		if (levelData[worldToTileY(player->y)][worldToTileX(player->x - player->width*OBJECT_SIZE)]){
			player->collidedLeft = true;
			player->x -= player->velocity_x * FIXED_TIMESTEP;
			player->velocity_x = 0.0f; 
			player->acceleration_x = 0.0f;
		}
	}
	else if (player->velocity_x > 0){
		if (levelData[worldToTileY(player->y)][worldToTileX(player->x + player->width*OBJECT_SIZE)]){
			player->collidedRight = true;
			player->x -= player->velocity_x * FIXED_TIMESTEP;
			player->velocity_x = 0.0f; 
			player->acceleration_x = 0.0f;
		}
	}
	else {
		player->collidedLeft = false;
		player->collidedRight = false;
	}

	//for (GLuint i = 0; i < map.size(); i++) {
	//	if (player->collidesWithX(map[i])){
	//		if (player->collidedLeft){ player->x = map[i]->x + map[i]->width*0.2f + player->width*0.2f; player->velocity_x = 0.0f; player->collidedLeft = false; }
	//		else if (player->collidedRight){ player->x = map[i]->x - map[i]->width*0.2f - player->width*0.2f; player->velocity_x = 0.0f; player->collidedRight = false; }
	//	}
	//}

	//for (GLuint j = 0; j < enemies.size(); j++) {
	//	enemies[j]->velocity_x += enemies[j]->acceleration_x * FIXED_TIMESTEP;
	//	enemies[j]->x += enemies[j]->velocity_x * FIXED_TIMESTEP;
	//	for (GLuint i = 0; i < map.size(); i++) {
	//		if (enemies[j]->collidesWithX(map[i])){
	//			if (enemies[j]->collidedLeft){ enemies[j]->x = map[i]->x + map[i]->width*0.2f + enemies[j]->width*0.2f; enemies[j]->velocity_x = 0.5f; enemies[j]->collidedLeft = false; }
	//			else if (enemies[j]->collidedRight){ enemies[j]->x = map[i]->x - map[i]->width*0.2f - enemies[j]->width*0.2f; enemies[j]->velocity_x = -0.5f; enemies[j]->collidedRight = false; }
	//		}
	//	}
	//	if (enemies[j]->collidesWithX(player)){
	//		ResetGame();
	//		break;
	//	}

	//	enemies[j]->velocity_y = lerp(enemies[j]->velocity_y, 0.0f, FIXED_TIMESTEP * enemies[j]->friction_y);
	//	enemies[j]->velocity_y += enemies[j]->acceleration_y * FIXED_TIMESTEP;
	//	enemies[j]->y += enemies[j]->velocity_y * FIXED_TIMESTEP;
	//	if (!enemies[j]->collidedBottom){ enemies[j]->acceleration_y = -2.0f; }//gravity
	//	for (GLuint i = 0; i < map.size(); i++) {
	//		if (enemies[j]->collidesWithY(map[i])){
	//			if (enemies[j]->collidedBottom){ enemies[j]->y = map[i]->y + map[i]->height*0.2f + enemies[j]->height*0.2f; enemies[j]->velocity_y = 0.0f; enemies[j]->collidedBottom = false; }
	//			else if (enemies[j]->collidedTop){ enemies[j]->y = map[i]->y - map[i]->height*0.2f - enemies[j]->height*0.2f; enemies[j]->velocity_y = 0.0f; enemies[j]->collidedTop = false; }
	//		}
	//	}


	//	if (enemies[j]->collidesWithY(player)){
	//		ResetGame();
	//		break;
	//	}
	//}

}

void PlatformAdventure::Update(float elapsed)
{
	timePassed += elapsed;


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
	}
}


void PlatformAdventure::Render()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	if (score){ glTranslatef(-0.8f, 0.0f, 0.0f); PrintText(fontTexture, "YOU WIN!", 0.25f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f); }//win

	else{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(-player->x, -player->y, 0.0f); //scrolling
		renderMap();
		for (GLuint i = 0; i < enemies.size(); i++) { enemies[i]->DrawSprite(OBJECT_SIZE); }
		player->DrawSprite(OBJECT_SIZE);
		glPopMatrix();
	}
	SDL_GL_SwapWindow(displayWindow);
}

void PlatformAdventure::renderMap()
{
	//glLoadIdentity();
	//glTranslatef(-TILE_SIZE * mapWidth / 2, TILE_SIZE * mapHeight / 2, 0.0f);
	glBindTexture(GL_TEXTURE_2D, spriteSheet);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	vector<float> vertexData;
	vector<float> texCoordData;
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			if (levelData[y][x] != 0) {
				float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
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
	glDrawArrays(GL_QUADS, 0, vertexData.size()/2);

}
