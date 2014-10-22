#include "GameObject.h"


GameObject::GameObject(int spriteSheet, float posX, float posY, float dX, float dY, float rot, float uSprite, float vSprite, float w, float h, float m, bool immovable, bool collidable)
{
	textureID = spriteSheet;
	x = posX;
	y = posY;
	velocity_x = dX;
	velocity_y = dY;
	acceleration_x = 0.0f;
	acceleration_y = 0.0f;
	friction_x = 1.0f;
	friction_y = 1.0f;
	rotation = rot;
	u = uSprite;
	v = vSprite;
	width = w;
	height = h;
	mass = m;
	isStatic = immovable;
	enableCollisions = collidable;
	collidedBottom = false;
	collidedTop = false;
	collidedRight = false;
	collidedLeft = false;
}



GameObject::~GameObject()
{
}


void GameObject::DrawSprite(float scale)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	glPushMatrix();
	glTranslatef(x, y, 0.0);
	glRotatef(rotation, 0.0, 0.0, 1.0);
	GLfloat quad[] = { -width * scale, height * scale, -width * scale, -height * scale,
		width * scale, -height * scale, width * scale, height * scale };
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] = { u, v, u, v + height, u + width, v + height, u + width, v };
	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

bool GameObject::collidesWithY(GameObject *other){
	if (y - height*0.2f < other->y + other->height*0.2f && y - height*0.2f > other->y - other->height*0.2f &&
		((x - width*0.2f < other->x + other->width*0.2f&&x - width*0.1f > other->x - other->width*0.2f) ||
		(x + width*0.2f > other->x - other->width*0.2f&&x + width*0.1f < other->x + other->width*0.2f))){
		collidedBottom = true;
		/*y = other->y + other->height*0.2f + height*0.2f;
		velocity_y = 0.0f;*/
		return true;
	}
	else if (y + height*0.2f < other->y + other->height*0.2f && y + height*0.2f > other->y - other->height*0.2f &&
		((x - width*0.2f < other->x + other->width*0.2f&&x - width*0.1f > other->x - other->width*0.2f) ||
		(x + width*0.2f > other->x - other->width*0.2f&&x + width*0.1f < other->x + other->width*0.2f))){
		collidedTop = true;
		return true;
	}
	return false;
}

bool GameObject::collidesWithX(GameObject *other){
	if (x - width*0.2f < other->x + other->width*0.2f&&x - width*0.2f > other->x - other->width*0.2f &&
		((y - height*0.1f < other->y + other->height*0.2f&&y - height*0.1f > other->y - other->height*0.2f) ||
		(y + height*0.1f < other->y + other->height*0.2f&&y + height*0.1f > other->y - other->height*0.2f))){
		collidedLeft = true;
		return true;
	}
	else if (x + width*0.2f < other->x + other->width*0.2f&&x + width*0.2f > other->x - other->width*0.2f &&
		((y - height*0.2f < other->y + other->height*0.2f&&y - height*0.2f > other->y - other->height*0.2f) ||
		(y + height*0.2f < other->y + other->height*0.2f&&y + height*0.2f > other->y - other->height*0.2f))){
		collidedRight = true;
		return true;
	}
	return false;
}
