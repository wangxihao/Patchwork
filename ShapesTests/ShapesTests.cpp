// ShapesTests.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Shape_test.h"
#include "SDL2\SDL.h"

using namespace Patchwork;
int _tmain(int argc, _TCHAR* argv[])
{
	
	Shape_test::run_tests();
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Event event;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_CreateWindowAndRenderer(800, 600, 0, &window, &renderer);

	Circle c = Circle(Vec2(400, 300), 50, Color(255, 0, 0));
	Polygon p = Polygon({ { 500, 200 }, { 550, 200 }, { 550, 250 }, { 500, 250 } }, Color(0, 0, 255));
	Line l = Line(Vec2(400, 300), Vec2(100, 100), Color(255, 128, 50));
	Ellipse e = Ellipse(Vec2(600, 500), Vec2(100, 50), Color(0, 255, 0));
	while (1) {
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) {
			break;
		}
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0x00);
		SDL_RenderClear(renderer);
		c.display(renderer);
		p.display(renderer);
		l.display(renderer);
		e.display(renderer);
		SDL_RenderPresent(renderer);
	}
	
	return 0;
}
