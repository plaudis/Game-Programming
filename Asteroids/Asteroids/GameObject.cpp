#include "GameObject.h"

#define OBJECT_SIZE 0.1f

GameObject::GameObject(int sprite, float posX, float posY, float w, float h, float vx, float vy, float rot, float dR, float m)
{
	textureID = sprite;
	x = posX;
	y = posY;
	width = w;
	height = h;
	velocity_x = vx;
	velocity_y = vy;
	acceleration = 0.0f;
	rotationDegrees = rot;
	rotationVelocity = dR;
	rotationAcceleration = 0.0f;
	mass = m;
	collidedBottom = false;
	collidedTop = false;
	collidedRight = false;
	collidedLeft = false;
}



GameObject::~GameObject()
{
}


void GameObject::DrawSprite()
{
	buildMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glMatrixMode(GL_MODELVIEW);
	
	glPushMatrix();
	//glLoadIdentity();
	//glTranslatef(x, y, 0.0);
	//glRotatef(rotationDegrees, 0.0, 0.0, 1.0);
	glMultMatrixf(matrix.ml);
	GLfloat quad[] = { -width * OBJECT_SIZE, height * OBJECT_SIZE, -width * OBJECT_SIZE, -height * OBJECT_SIZE,
		width * OBJECT_SIZE, -height * OBJECT_SIZE, width * OBJECT_SIZE, height * OBJECT_SIZE };
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
	scale.scale(width, height);
	translate.translate(x, y, 0.0);
	rotate.rotate(rotationDegrees);
	matrix = scale*rotate*translate;
}

bool GameObject::collidesWith(GameObject *other){
	buildMatrix();
	Vector vertex1(other->width*OBJECT_SIZE, other->height*OBJECT_SIZE, 0.0f);
	Vector vertex2(-other->width*OBJECT_SIZE, other->height*OBJECT_SIZE, 0.0f);
	Vector vertex3(-other->width*OBJECT_SIZE, -other->height*OBJECT_SIZE, 0.0f);
	Vector vertex4(other->width*OBJECT_SIZE, -other->height*OBJECT_SIZE, 0.0f);
	vertex1 = (other->matrix)*vertex1;
	vertex2 = (other->matrix)*vertex2;
	vertex3 = (other->matrix)*vertex3;
	vertex4 = (other->matrix)*vertex4;

	vertex1 = matrix.inverse()*vertex1;
	vertex2 = matrix.inverse()*vertex2;
	vertex3 = matrix.inverse()*vertex3;
	vertex4 = matrix.inverse()*vertex4;

	Vector normalX(1.0f,0.0f,0.0f);
	Vector normalY(0.0f, 1.0f, 0.0f);

	std::vector<float> x;
	std::vector<float> y;
	x.push_back(normalX.dot(vertex1));
	x.push_back(normalX.dot(vertex2));
	x.push_back(normalX.dot(vertex3));
	x.push_back(normalX.dot(vertex4));
	std::sort(x.begin(), x.end());
	y.push_back(normalY.dot(vertex1));
	y.push_back(normalY.dot(vertex2));
	y.push_back(normalY.dot(vertex3));
	y.push_back(normalY.dot(vertex4));
	std::sort(y.begin(), y.end());

	if (((height*OBJECT_SIZE > y[0] && height*OBJECT_SIZE<y[3]) || (-height*OBJECT_SIZE>y[0] && -height*OBJECT_SIZE < y[3]))&&
		((width*OBJECT_SIZE > x[0] && width*OBJECT_SIZE<x[3]) || (-width*OBJECT_SIZE>x[0] && -width*OBJECT_SIZE < x[3]))){
		return true;
	}
	return false;
}