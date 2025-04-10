#include <stdio.h>
#include <fstream>
#include <SDL2/SDL_syswm.h>
#include <stb_image.h>
#include "game.h"
#include "gfx/lowlevel/opengl_arb/opengl_backend.h"
#include "gfx/lowlevel/d3d9/d3d9_backend.h"
#include <vector>
#include "global_res.h"
#include "util/string_utils.h"
#include "util/logging.h"

#include "main_game/main_game.h"
#include "testing/u16_test.h"
#include "testing/input_test.h"
#include "testing/gfx_backend_test.h"
#include "dev/state_selector.h"

#define LOG_INFO(message) Logging::LogInfo("Game", message)
#define LOG_INFO_ARGS(message, ...) Logging::LogInfo("Game", message, __VA_ARGS__)
#define LOG_WARN(message) Logging::LogWarn("Game", message)
#define LOG_WARN_ARGS(message, ...) Logging::LogWarn("Game", message, __VA_ARGS__)
#define LOG_ERROR(message) Logging::LogError("Game", message)
#define LOG_ERROR_ARGS(message, ...) Logging::LogError("Game", message, __VA_ARGS__)

Game::Game()
{
	windowWidth = 1280;
	windowHeight = 720;
	windowTitle = "DIVA";

	currentState = nullptr;
}

Game::~Game()
{
}

void Game::Quit()
{
	Logging::LogMessage("--- Exiting...");
	running = false;
}

static void convertBuildDateToVersion(int* year, int* month)
{
	char buildDate[] = "1970.01.01T00:00:00";
	
	char* buildYearText = buildDate;
	char* buildMonthText = SDL_strchr(buildYearText, '.') + 1;

	int buildYear = SDL_atoi(buildYearText);
	int buildMonth = SDL_atoi(buildMonthText);

	*year = buildYear - 2000;
	*month = buildMonth;
}

static int buildYear = 0;
static int buildMonth = 0;

bool Game::Initialize()
{
	if (initialized)
	{
		return true;
	}

	convertBuildDateToVersion(&buildYear, &buildMonth);

	Logging::LogMessage("--- Starshine %02d.%02d ---", buildYear, buildMonth);
	Logging::LogMessage("--- Starting...");

	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0)
	{
		LOG_ERROR_ARGS("Failed to initialize SDL2. Error: %s", SDL_GetError());
		Logging::LoggingQuit();
		return false;
	}

	SDL_version sdlVersion;
	SDL_GetVersion(&sdlVersion);

	LOG_INFO_ARGS("SDL Version: %u.%u.%u", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);
	LOG_INFO_ARGS("SDL Platform: %s", SDL_GetPlatform());
	//Logging::LogMessage("SDL Version: %u.%u.%u", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);

	u32 windowFlags = SDL_WINDOW_SHOWN;

	fileSystem = FileSystem::GetInstance();
	if (fileSystem->MountPath("diva") != true)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Main content directory \"diva\" does not exist.", NULL);
		return false;
	}

	loadInitialConfig();

	if (gfxBackendType == GFXBackendType::BACKEND_OPENGL_ARB)
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		windowFlags |= SDL_WINDOW_OPENGL;
	}
	
	if (fullscreen)
	{
		windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, windowFlags);
	if (window == NULL)
	{
		Logging::LogError("Init", "Failed to create game window. Error: %s", SDL_GetError());
		SDL_Quit();
		return false;
	}

	setWindowIcon();

	switch (gfxBackendType)
	{
	case GFXBackendType::BACKEND_OPENGL_ARB:
		graphicsBackend = new GFX::LowLevel::OpenGL_ARB::Backend_OpenGL();
		break;
	case GFXBackendType::BACKEND_D3D9:
		graphicsBackend = new GFX::LowLevel::D3D9::Backend_D3D9();
		break;
	}

	if (!graphicsBackend->Initialize(window))
	{
		return false;
	}

	keyboardState = Input::Keyboard::GetInstance();
	mouseState = Input::Mouse::GetInstance();

	//SDL_CaptureMouse(SDL_TRUE);

	audioEngine = AudioEngine::GetInstance();
	audioEngine->Initialize();

	GlobalResources::Load(graphicsBackend);

	initialized = true;
	running = true;
	firstFrame = true;
	return true;
}

bool Game::Loop()
{
	if (!initialized)
	{
		return false;
	}

	while (running)
	{
		ticks_now = (SDL_GetPerformanceCounter() * 1000.0 / (double)SDL_GetPerformanceFrequency());
		ticks_delta = ticks_now - ticks_lastFrame;
		ticks_lastFrame = ticks_now;

		deltaTime_ms = ticks_delta;
		
		keyboardState->NextFrame();
		mouseState->NextFrame();

		while (SDL_PollEvent(&sdlEvent))
		{
			switch (sdlEvent.type)
			{
				case SDL_QUIT:
					Quit();
					break;
				case SDL_WINDOWEVENT:
				{
					switch (sdlEvent.window.event)
					{
					case SDL_WINDOWEVENT_RESTORED:
						active = true;
						break;
					case SDL_WINDOWEVENT_MINIMIZED:
						active = false;
						break;
					case SDL_WINDOWEVENT_RESIZED:
						windowWidth = sdlEvent.window.data1;
						windowHeight = sdlEvent.window.data2;
						graphicsBackend->ResizeMainRenderTarget(windowWidth, windowHeight);
						if (currentState != nullptr)
						{
							currentState->OnResize(windowWidth, windowHeight);
						}
						break;
					}
					break;
				}
			}

			keyboardState->Poll(sdlEvent);
			mouseState->Poll(sdlEvent);
		}

		if (running && !firstFrame)
		{
			if (keyboardState->IsKeyTapped(SDL_SCANCODE_F6) && currentGameState != GameStates::DEVSTATE_STATE_SELECTOR)
			{
				SetState(GameStates::DEVSTATE_STATE_SELECTOR);
			}

			if (changeState)
			{
				if (currentState != nullptr)
				{
					currentState->UnloadContent();
					currentState->Destroy();
					delete currentState;
					currentState = nullptr;
				}

				currentState = nextState;
				nextState = nullptr;

				if (currentState->Initialize() != true)
				{
					Logging::LogError("Game", "Failed to initialize a game state");
					return false;
				}
				LOG_INFO("Game state initialized");

				if (currentState->LoadContent() != true)
				{
					Logging::LogError("Game", "Game state failed to load content");
					return false;
				}
				LOG_INFO("Game state content loaded");

				changeState = false;
			}

			if (currentState != nullptr)
			{
				if (!changeState && currentState != nullptr)
				{
					currentState->Update();
				}
				if (!changeState && currentState != nullptr)
				{
					currentState->Draw();
				}
			}
		}

		firstFrame = false;
	}

	SDL_DestroyWindow(window);

	if (currentState != nullptr)
	{
		currentState->UnloadContent();
		currentState->Destroy();
		LOG_INFO("State destroyed");
	}

	GlobalResources::Destroy();
	graphicsBackend->Destroy();
	audioEngine->Destroy();

	delete[] graphicsBackend;
	SDL_Quit();
	return true;
}

/*void Game::SetState(GameState* state)
{
	if (state != nullptr)
	{
		if (currentState != nullptr)
		{
			currentState->UnloadContent();
			currentState->Destroy();
			LOG_INFO("Previous state destroyed");
		}

		currentState = state;
		currentState->game = this;
		currentState->fileSystem = fileSystem;
		currentState->graphicsBackend = graphicsBackend;
		currentState->keyboardState = keyboardState;
		currentState->mouseState = mouseState;
		currentState->audio = audioEngine;
		changeState = true;

		currentGameState = GameStates::STATE_UNREGISTIRED;
	}
}*/

void Game::SetState(GameStates state)
{
	if (currentState != nullptr)
	{
		LOG_INFO_ARGS("Chaning game state: [%s] -> [%s]\n",
		GameStateNames[static_cast<int>(currentGameState)], GameStateNames[static_cast<int>(state)]);
	}

	GameState* stateClass = nullptr;

	switch (state)
	{
		case GameStates::STATE_MAINGAME:
			stateClass = new MainGame::MainGameState();
			break;
		case GameStates::DEVSTATE_U16_TEST:
			stateClass = new Testing::U16Test();
			break;
		case GameStates::DEVSTATE_INPUT_TEST:
			stateClass = new Testing::InputTest();
			break;
		case GameStates::DEVSTATE_GFX_BACKEND_TEST:
			stateClass = new Testing::GFXBackendTest();
			break;
		case GameStates::DEVSTATE_STATE_SELECTOR:
			stateClass = new Dev::StateSelector();
			break;
	}

	if (stateClass != nullptr)
	{
		nextState = stateClass;
		nextState->game = this;
		nextState->fileSystem = fileSystem;
		nextState->graphicsBackend = graphicsBackend;
		nextState->keyboardState = keyboardState;
		nextState->mouseState = mouseState;
		nextState->audio = audioEngine;
		changeState = true;

		currentGameState = state;
	}
}

bool Game::IsActive()
{
	return active;
}

void Game::GetVersionNumber(int* year, int* month)
{
	*year = buildYear;
	*month = buildMonth;
}

const char* Game::GetPlatformName()
{
#if defined(_WIN32)
	return "Windows";
#endif
	return "Unknown"; 
}

void Game::loadInitialConfig()
{
	std::filesystem::path configPath = fileSystem->GetSaveDataFilePath("initconfig.txt", false);
	if (!std::filesystem::exists(configPath))
	{
		Logging::LogWarn("Init", "Initial config does not exist. Writing...");
		writeInitialConfig();
	}

	std::fstream configFile;
	configFile.open(configPath, std::ios::in | std::ios::binary);

	std::vector<string> keyValue;

	for (std::string line; std::getline(configFile, line);)
	{
		if (line.length() > 1)
		{
			keyValue = Utils::String::Split(line, " = ");

			if (keyValue.size() == 2)
			{
				if (keyValue[0] == "WindowWidth")
				{
					windowWidth = std::stoi(keyValue[1]);
				}
				else if (keyValue[0] == "WindowHeight")
				{
					windowHeight = std::stoi(keyValue[1]);
				}
				else if (keyValue[0] == "WindowFullscreen")
				{
					if (keyValue[1] == "true")
					{
						fullscreen = true;
					}
					else if (keyValue[1] == "false")
					{
						fullscreen = false;
					}
				}
				else if (keyValue[0] == "GraphicsBackend")
				{
					if (keyValue[1] == "OpenGL_ARB")
					{
						gfxBackendType = GFXBackendType::BACKEND_OPENGL_ARB;
					}
					else if (keyValue[1] == "D3D9")
					{
						gfxBackendType = GFXBackendType::BACKEND_D3D9;
					}
					else
					{
						Logging::LogWarn("Init", "Unknown graphics backend. Defaulting to OpenGL_ARB.");
					}
				}
				else
				{
					Logging::LogWarn("Init", "Unknown config parameter \"%s\"", keyValue[0].c_str());
				}
			}

			keyValue.clear();
		}
	}

	configFile.close();
}

void Game::writeInitialConfig()
{
	std::filesystem::path configPath = fileSystem->GetSaveDataFilePath("initconfig.txt", false);

	SDL_Rect displayBounds = {};
	SDL_GetDisplayBounds(0, &displayBounds);

	std::fstream configFile;
	configFile.open(configPath, std::ios::out | std::ios::binary);

	configFile << "WindowWidth = " << std::to_string(displayBounds.w) << "\n";
	configFile << "WindowHeight = " << std::to_string(displayBounds.h) << "\n";
	configFile << "WindowFullscreen = " << "true" << "\n";
	configFile << "GraphicsBackend = " << "OpenGL_ARB" << "\n";

	configFile.close();
}

void Game::setWindowIcon()
{
	std::filesystem::path windowIconPath = fileSystem->GetContentFilePath("resources/gameicon.png");
	if (std::filesystem::exists(windowIconPath))
	{
		std::fstream iconFile;
		size_t iconFileSize = std::filesystem::file_size(windowIconPath);

		iconFile.open(windowIconPath, std::ios::in | std::ios::binary);
		u8* iconData = new u8[iconFileSize];
		iconFile.read((char*)iconData, iconFileSize);
		iconFile.close();

		int iconWidth, iconHeight, iconChannels;
		u8* iconDecodedData = stbi_load_from_memory(iconData, iconFileSize, &iconWidth, &iconHeight, &iconChannels, 4);

		if (iconDecodedData != NULL)
		{
			SDL_Surface* iconSurface = SDL_CreateRGBSurfaceWithFormatFrom(
				iconDecodedData, 
				iconWidth, 
				iconHeight, 
				32, 
				iconWidth * 4, 
				SDL_PIXELFORMAT_ABGR8888);

			SDL_SetWindowIcon(window, iconSurface);
			SDL_FreeSurface(iconSurface);
			stbi_image_free(iconDecodedData);
		}

		delete[] iconData;
	}
}

