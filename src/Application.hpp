#pragma once

#include <SDL3/SDL.h>
#include "Chip8.hpp"

class Application {
public:
	int Init();
	void Run();
	void Shutdown();
	void Draw();
private:
	Chip8 m_chip8;
	SDL_Window* m_window = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	SDL_Texture* m_texture = nullptr;

	void HandleKeyUp(SDL_Keycode key);
	void HandleKeyDown(SDL_Keycode key);
};