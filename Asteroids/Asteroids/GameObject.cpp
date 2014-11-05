#include "GameObject.h"

GameObject::GameObject(int sprite, float posX, float posY, float w, float h, float v, float rot, float dR, float m)
{
	textureID = sprite;
	x = posX;
	y = posY;
	width = w;
	height = h;
	velocity = v;
	acceleration = 0.0f;
	rotationDegrees = rot;
	rotationVelocity = 0.0f;
	rotationAcceleration = dR;
	mass = m;
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
	buildMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	glPushMatrix();
	//glTranslatef(x, y, 0.0);
	//glRotatef(rotationDegrees, 0.0, 0.0, 1.0);
	glMultMatrixf(matrix.ml);
	GLfloat quad[] = { -width * scale, height * scale, -width * scale, -height * scale,
		width * scale, -height * scale, width * scale, height * scale };
	glVertexPointer(2, GL_FLOAT, 0, quad);
	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat quadUVs[] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0 };
	glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void GameObject::buildMatrix(){
	Matrix scale, translate, rotate;
	scale.m[0][0] = width;
	scale.m[1][1] = height;
	translate.m[0][3] = x;
	translate.m[1][3] = y;
	rotate.m[0][0] = cos(rotationDegrees*PI / 180.0f);
	rotate.m[0][1] = -sin(rotationDegrees*PI / 180.0f);
	rotate.m[1][0] = sin(rotationDegrees*PI / 180.0f);
	rotate.m[1][1] = cos(rotationDegrees*PI / 180.0f);
	matrix = scale*rotate*translate;
}

bool GameObject::collidesWith(GameObject *other){
	//if (y - height*0.2f < other->y + other->height*0.2f && y - height*0.2f > other->y - other->height*0.2f &&
	//	((x - width*0.2f < other->x + other->width*0.2f&&x - width*0.1f > other->x - other->width*0.2f) ||
	//	(x + width*0.2f > other->x - other->width*0.2f&&x + width*0.1f < other->x + other->width*0.2f))){
	//	collidedBottom = true;
	//	/*y = other->y + other->height*0.2f + height*0.2f;
	//	velocity_y = 0.0f;*/
	//	return true;
	//}
	//else if (y + height*0.2f < other->y + other->height*0.2f && y + height*0.2f > other->y - other->height*0.2f &&
	//	((x - width*0.2f < other->x + other->width*0.2f&&x - width*0.1f > other->x - other->width*0.2f) ||
	//	(x + width*0.2f > other->x - other->width*0.2f&&x + width*0.1f < other->x + other->width*0.2f))){
	//	collidedTop = true;
	//	return true;
	//}
	return false;
}