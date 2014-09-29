#include <SpaceInvader.h>

int main(int argc, char *argv[])
{
	SpaceInvader game;
	while (!game.UpdateAndRender()) {}
	return 0;
}