
#include "PlatformAdventure.h"

int main(int argc, char *argv[])
{
	PlatformAdventure game;
	while (!game.UpdateAndRender()) {}
	return 0;
}


//#include <SDL.h>
//#include <SDL_opengl.h>
//#include <SDL_image.h>
//#include <SDL_mixer.h>
//
//SDL_Window* displayWindow;
//
//int main(int argc, char *argv[])
//{
//	SDL_Init(SDL_INIT_VIDEO);
//	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
//	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
//	SDL_GL_MakeCurrent(displayWindow, context);
//	//Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
//	bool done = false;
//	
//	SDL_Event event;
//
//
//
//	while (!done) {
//		while (SDL_PollEvent(&event)) {
//			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
//				done = true;
//			}
//		}
//		SDL_GL_SwapWindow(displayWindow);
//	}
//
//	SDL_Quit();
//	return 0;
//}