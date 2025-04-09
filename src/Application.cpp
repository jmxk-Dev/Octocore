#include "Application.hpp"

//#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

constexpr int SCREEN_SCALE = 10;
constexpr int SCR_WIDTH = 64;
constexpr int SCR_HEIGHT = 32;

uint32_t videoBuffer[SCR_WIDTH * SCR_HEIGHT];

int Application::Init() {
    const bool sdlSuccess = SDL_Init(SDL_INIT_VIDEO);
    if (!sdlSuccess)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

	SDL_CreateWindowAndRenderer("Chip-8 Emulator", SCR_WIDTH * SCREEN_SCALE, SCR_HEIGHT * SCREEN_SCALE, 0, &m_window, &m_renderer);

	if (!m_window) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL Window creation failed: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	if (!m_renderer) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL Renderer creation failed: %s", SDL_GetError());
		SDL_DestroyWindow(m_window);
		SDL_Quit();
		return -1;
	}

	m_texture = SDL_CreateTexture(m_renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCR_WIDTH, SCR_HEIGHT);

	if (!m_texture) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL Texture creation failed: %s", SDL_GetError());
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
		return -1;
	}

    return 0;
}

void Application::Run() {
	m_chip8.Init();
	m_chip8.LoadGame("/roms/pong2.c8");

	bool isGameRunning = true;
	while (isGameRunning) {
		// Emulate one cycle
		m_chip8.EmulateCycle();

		// Draw if 0x00E0 or 0xDXYN
		if (m_chip8.GetDrawFlag()) {
			Draw();
			m_chip8.SetDrawFlag(false);
		}

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_EVENT_QUIT:
				isGameRunning = false;
				break;
			case SDL_EVENT_KEY_DOWN:
				if (event.key.key == SDLK_ESCAPE)
					isGameRunning = false;
				HandleKeyDown(event.key.key);
				break;
			case SDL_EVENT_KEY_UP:
				HandleKeyUp(event.key.key);
				break;
			}
		}
	}
}

void Application::Draw() {
	for (int i = 0; i < SCR_WIDTH * SCR_HEIGHT; ++i) {
		videoBuffer[i] = m_chip8.gfx[i] ? 0xFFFFFFFF : 0xFF000000;
	}

	SDL_UpdateTexture(m_texture, nullptr, videoBuffer, SCR_WIDTH * sizeof(uint32_t));
	SDL_RenderClear(m_renderer);
	SDL_RenderTexture(m_renderer, m_texture, nullptr, nullptr);
	SDL_RenderPresent(m_renderer);
}

void Application::Shutdown() {
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}

void Application::HandleKeyDown(SDL_Keycode key) {
	switch (key) {
	case SDLK_1: m_chip8.key[0x1] = 1; break;
	case SDLK_2: m_chip8.key[0x2] = 1; break;
	case SDLK_3: m_chip8.key[0x3] = 1; break;
	case SDLK_4: m_chip8.key[0xC] = 1; break;
	case SDLK_Q: m_chip8.key[0x4] = 1; break;
	case SDLK_W: m_chip8.key[0x5] = 1; break;
	case SDLK_E: m_chip8.key[0x6] = 1; break;
	case SDLK_R: m_chip8.key[0xD] = 1; break;
	case SDLK_A: m_chip8.key[0x7] = 1; break;
	case SDLK_S: m_chip8.key[0x8] = 1; break;
	case SDLK_D: m_chip8.key[0x9] = 1; break;
	case SDLK_F: m_chip8.key[0xE] = 1; break;
	case SDLK_Y: m_chip8.key[0xA] = 1; break;
	case SDLK_X: m_chip8.key[0x0] = 1; break;
	case SDLK_C: m_chip8.key[0xB] = 1; break;
	case SDLK_V: m_chip8.key[0xF] = 1; break;
	}
}

void Application::HandleKeyUp(SDL_Keycode key) {
	switch (key) {
	case SDLK_1: m_chip8.key[0x1] = 0; break;
	case SDLK_2: m_chip8.key[0x2] = 0; break;
	case SDLK_3: m_chip8.key[0x3] = 0; break;
	case SDLK_4: m_chip8.key[0xC] = 0; break;
	case SDLK_Q: m_chip8.key[0x4] = 0; break;
	case SDLK_W: m_chip8.key[0x5] = 0; break;
	case SDLK_E: m_chip8.key[0x6] = 0; break;
	case SDLK_R: m_chip8.key[0xD] = 0; break;
	case SDLK_A: m_chip8.key[0x7] = 0; break;
	case SDLK_S: m_chip8.key[0x8] = 0; break;
	case SDLK_D: m_chip8.key[0x9] = 0; break;
	case SDLK_F: m_chip8.key[0xE] = 0; break;
	case SDLK_Y: m_chip8.key[0xA] = 0; break;
	case SDLK_X: m_chip8.key[0x0] = 0; break;
	case SDLK_C: m_chip8.key[0xB] = 0; break;
	case SDLK_V: m_chip8.key[0xF] = 0; break;
	}
}