#pragma once
#include <string>
#include <SDL2/SDL.h>
#include "common/int_types.h"
#include "io/filesystem.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "gfx/lowlevel/backend.h"
#include "audio/audio.h"

using std::string;

using IO::FileSystem;
using Input::Keyboard;
using Input::Mouse;
using Audio::AudioEngine;

using GFXBackend = GFX::LowLevel::Backend;
using GFXBackendType = GFX::LowLevel::BackendType;

class Game;
class GameState;

enum class GameStates
{
	STATE_UNREGISTIRED = -2,
	STATE_NONE = -1,

	STATE_MAINGAME,

	DEVSTATE_U16_TEST,
	DEVSTATE_INPUT_TEST,
	DEVSTATE_GFX_BACKEND_TEST,

	DEVSTATE_STATE_SELECTOR,

	STATE_COUNT
};

constexpr const char* GameStateNames[static_cast<int>(GameStates::STATE_COUNT)] = 
{
	"Main Game",
	"[Dev] UTF-16 Text Test",
	"[Dev] Input Test",
	"[Dev] Graphics Backend Test",
	"[Dev] State Selector"
};

class Game
{
public:
	/* SDL-related properties */

	SDL_Window* window;
	int windowWidth, windowHeight;
	string windowTitle;

	/* Graphics rendering */

	GFXBackendType gfxBackendType = GFXBackendType::BACKEND_OPENGL_ARB;
	bool fullscreen = false;

	/* State properties */

	bool firstFrame;
	bool running;
	SDL_Event sdlEvent;

	/* Timing */

	double ticks_lastFrame = 0.0;
	double ticks_now = 0.0;
	double ticks_delta = 0.0;
	double deltaTime_ms = 0.0;
	double actualFrameTime_ms = 0.0;

	/* Game state list */
	GameStates currentGameState = GameStates::STATE_NONE;
	GameState* stateList[static_cast<int>(GameStates::STATE_COUNT)] = {};

	/* Constructor and destructor */

	Game();
	~Game();
	
	/* Public functions */

	void Quit();

	bool Initialize();
	bool Loop();
	//void SetState(GameState* state);
	void SetState(GameStates state);

	bool IsActive();

	void GetVersionNumber(int* year, int* month);
	const char* GetPlatformName();
private:
	bool initialized = false;
	bool active = true;

	/* File system */
	FileSystem* fileSystem;
	GFXBackend* graphicsBackend;

	/* Audio engine */
	AudioEngine* audioEngine;

	/* Game state */
	bool changeState;
	GameState* currentState;
	GameState* nextState;

	/* Input */
	Input::Keyboard* keyboardState;
	Input::Mouse* mouseState;

	void loadInitialConfig();
	void writeInitialConfig();
	void setWindowIcon();
};

class GameState
{
public:
	Game* game = nullptr;
	FileSystem* fileSystem = nullptr;
	Input::Keyboard* keyboardState = nullptr;
	Input::Mouse* mouseState = nullptr;
	GFXBackend* graphicsBackend = nullptr;
	AudioEngine* audio = nullptr;

	GameState() { };

	virtual bool Initialize() = 0;
	virtual bool LoadContent() = 0;
	virtual void UnloadContent() = 0;
	virtual void Destroy() = 0;

	virtual void OnResize(u32 newWidth, u32 newHeight) = 0;

	virtual void Update() = 0;
	virtual void Draw() = 0;
};
