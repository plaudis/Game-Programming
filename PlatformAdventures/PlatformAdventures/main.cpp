
#include "PlatformAdventure.h"

int main(int argc, char *argv[])
{
	PlatformAdventure game;
	while (!game.UpdateAndRender()) {}
	return 0;
}