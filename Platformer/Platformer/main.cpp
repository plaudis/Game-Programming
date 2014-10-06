#include "Platformer.h"

int main(int argc, char *argv[])
{
	Platformer game;
	while (!game.UpdateAndRender()) {}
	return 0;
}