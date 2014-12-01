#include "Alienated.h"

int main(int argc, char *argv[])
{
	Alienated game;
	while (!game.UpdateAndRender()) {}
	return 0;
}