#include "GameObject.h"


GameObject::GameObject(int texture, float posX, float posY, float rot, float w, float h)
{
	textureID = texture;
	x = posX;
	y = posY;
	rotation = rot;
	width = w;
	height = h;
}

GameObject::GameObject(int spriteSheet, float posX, float posY,float dX, float dY, float rot, float uSprite, float vSprite, float w, float h)
{
	textureID = spriteSheet;
	x = posX;
	y = posY;
	direction_x = dX;
	direction_y = dY;
	speed = 1.0f;
	rotation = rot;
	u = uSprite;
	v = vSprite;
	width = w;
	height = h;
}



GameObject::~GameObject()
{
}

void GameObject::Draw()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(x, y, 0.0);
	glScalef(width, height, 1.0);
	glRotatef(rotation, 0.0, 0.0, 1.0);
	GLfloat quad[] = { -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f };
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0 };
	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisable(GL_TEXTURE_2D);
}

void GameObject::DrawSprite(float scale)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(x, y, 0.0);
	glRotatef(rotation, 0.0, 0.0, 1.0);
	GLfloat quad[] = { -width * scale, height * 2 * scale, -width * scale, -height * 2 * scale,
		width * scale, -height * 2 * scale, width * scale, height * 2 * scale };//spritesheet is 1:2 so height needs to be multiplied to keep proportion
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] = { u, v, u, v + height, u + width, v + height, u + width, v };
	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisable(GL_TEXTURE_2D);
}
