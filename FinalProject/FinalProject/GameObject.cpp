#include "GameObject.h"
#define OBJECT_SIZE 0.2f

GameObject::GameObject(int spriteSheet, float posX, float posY, float wi, float hi, float dX, float dY, float rot, float uSprite, float vSprite, float wj, float hj, int hit, bool flip)
{
	textureID = spriteSheet;
	x = posX;
	y = posY;
	width = wi;
	height = hi;
	velocity_x = dX;
	velocity_y = dY;
	acceleration_x = 0.0f;
	acceleration_y = 0.0f;
	friction_x = 1.0f;
	friction_y = 1.0f;
	rotation = rot;
	u = uSprite;
	v = vSprite;
	w = wj;
	h = hj;
	hits = hit;
	flipped = flip;
	bullets = 0;
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
	glPushMatrix();
	glTranslatef(x, y, 0.0);
	glRotatef(rotation, 0.0, 0.0, 1.0);
	float scale_x = 1.0f;
	float scale_y = 1.0f;
	GLfloat quad[] = { -width * scale*scale_x, height * scale*scale_y, -width * scale*scale_x, -height * scale*scale_y,
		width * scale*scale_x, -height * scale*scale_y, width * scale*scale_x, height * scale*scale_y };
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] = { u, v, u, v + h, u + w, v + h, u + w, v };
	if (flipped){ 
		quadUVs[0] = u + w; 
		quadUVs[1] = v; 
		quadUVs[2] = u + w; 
		quadUVs[3] = v + h; 
		quadUVs[4] = u; 
		quadUVs[5] = v + h; 
		quadUVs[6] = u; 
		quadUVs[7] = v;
	}
	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

bool GameObject::collidesWithY(GameObject *other){
	if (y - height*OBJECT_SIZE < other->y + other->height*OBJECT_SIZE && y - height*OBJECT_SIZE > other->y - other->height*OBJECT_SIZE &&
		((x - width*OBJECT_SIZE < other->x + other->width*OBJECT_SIZE&&x - width*OBJECT_SIZE > other->x - other->width*OBJECT_SIZE) ||
		(x + width*OBJECT_SIZE > other->x - other->width*OBJECT_SIZE&&x + width*OBJECT_SIZE < other->x + other->width*OBJECT_SIZE))){
		collidedBottom = true;
		return true;
	}
	else if (y + height*OBJECT_SIZE < other->y + other->height*OBJECT_SIZE && y + height*OBJECT_SIZE > other->y - other->height*OBJECT_SIZE &&
		((x - width*OBJECT_SIZE < other->x + other->width*OBJECT_SIZE&&x - width*OBJECT_SIZE > other->x - other->width*OBJECT_SIZE) ||
		(x + width*OBJECT_SIZE > other->x - other->width*OBJECT_SIZE&&x + width*OBJECT_SIZE < other->x + other->width*OBJECT_SIZE))){
		collidedTop = true;
		return true;
	}
	return false;
}

bool GameObject::collidesWithX(GameObject *other){
	if (x - width*OBJECT_SIZE < other->x + other->width*OBJECT_SIZE&&x - width*OBJECT_SIZE > other->x - other->width*OBJECT_SIZE &&
		((y - height*OBJECT_SIZE < other->y + other->height*OBJECT_SIZE&&y - height*OBJECT_SIZE > other->y - other->height*OBJECT_SIZE) ||
		(y + height*OBJECT_SIZE < other->y + other->height*OBJECT_SIZE&&y + height*OBJECT_SIZE > other->y - other->height*OBJECT_SIZE))){
		collidedLeft = true;
		return true;
	}
	else if (x + width*OBJECT_SIZE < other->x + other->width*OBJECT_SIZE&&x + width*OBJECT_SIZE > other->x - other->width*OBJECT_SIZE &&
		((y - height*OBJECT_SIZE < other->y + other->height*OBJECT_SIZE&&y - height*OBJECT_SIZE > other->y - other->height*OBJECT_SIZE) ||
		(y + height*OBJECT_SIZE < other->y + other->height*OBJECT_SIZE&&y + height*OBJECT_SIZE > other->y - other->height*OBJECT_SIZE))){
		collidedRight = true;
		return true;
	}
	return false;
}
