#include "particleEmitter.h"



particleEmitter::particleEmitter(float x, float y)
{
	posX = x;
	posY = y;
	for (int i = 0; i < 8; i++){ 
		particles.push_back(Particle()); 
		particles[i].posX = posX;
		particles[i].posY = posY;
		particles[i].lifetime = 1.0f;
		particles[i].vY = 2.0f*((float)rand() / (float)RAND_MAX);
		particles[i].vX = i/2.0f - 1.5f;
	}

}

particleEmitter::~particleEmitter()
{
}


void particleEmitter::Update(float elapsed)
{
	for (GLuint i = 0; i < particles.size(); i++) {
		particles[i].vY += (-2.0f) * elapsed;
		particles[i].posY += particles[i].vY * elapsed;

		particles[i].posX += particles[i].vX *elapsed;
	}
}

void particleEmitter::Render()
{
	std::vector<float> particleVertices;
	for (int i = 0; i < particles.size(); i++) {
		particleVertices.push_back(particles[i].posX);
		particleVertices.push_back(particles[i].posY);
	}
	glVertexPointer(2, GL_FLOAT, 0, particleVertices.data());
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_POINTS, 0, particleVertices.size() / 2);
}
