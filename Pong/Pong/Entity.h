#pragma once
class Entity
{
public:
	Entity();
	Entity(int texture, float posX,float posY, float rot, float w, float h);
	~Entity();

	void Draw();
	
	float x;
	float y;
	float rotation;
	int textureID;
	float width;
	float height;

	float speed;
	float direction_x;
	float direction_y;
};

