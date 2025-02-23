#pragma once
#include <string>
#include <SDL2/SDL.h>
#include "common/int_types.h"
#include "io/filesystem.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "gfx/lowlevel/backend.h"
#include "gfx/lowlevel/opengl_arb/opengl_backend.h"
#include "gfx/lowlevel/d3d9/d3d9_backend.h"

using std::string;

using IO::FileSystem;
using Input::Keyboard;
using Input::Mouse;

using GFXBackend = GFX::LowLevel::Backend;
using GFXBackendType = GFX::LowLevel::BackendType;
using GFX::LowLevel::OpenGL_ARB::Backend_OpenGL;

#ifdef _WIN32
#ifdef STARSHINE_GFX_D3D9
using GFX::LowLevel::D3D9::Backend_D3D9;
#endif
#endif

class Game;
class GameState;

enum class GameStates
{
	STATE_UNREGISTIRED = -2,
	STATE_NONE = -1,

	STATE_MAINGAME,

	DEVSTATE_U16_TEST,
	DEVSTATE_INPUT_TEST,

	DEVSTATE_STATE_SELECTOR,

	STATE_COUNT
};

const string GameStateNames[static_cast<int>(GameStates::STATE_COUNT)] = 
{
	"Main Game",
	"[Dev] UTF-16 Text Test",
	"[Dev] Input Test",
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

	u64 ticks_lastFrame = 0;
	u64 ticks_now = 0;
	u64 ticks_delta = 0;
	double deltaTime_ms = 0.0f;
	double actualFrameTime_ms = 0.0f;

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
	void SetState(GameState* state);
	void SetState(GameStates state);

	bool IsActive();

	void GetVersionNumber(int* year, int* month);
	const char* GetPlatformName();
private:
	bool initialized = false;
	bool active = true;

	/* File system */
	FileSystem* fileSystem;

	/* Graphics backends */
	Backend_OpenGL gfxBackend_OpenGL;
	
#ifdef _WIN32
#ifdef STARSHINE_GFX_D3D9
	Backend_D3D9 gfxBackend_D3D9;
#endif
#endif

	GFXBackend* graphicsBackend;

	/* Game state */
	bool changeState;
	GameState* currentState;

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

	GameState() { };

	virtual bool Initialize() = 0;
	virtual bool LoadContent() = 0;
	virtual void UnloadContent() = 0;
	virtual void Destroy() = 0;

	virtual void OnResize(u32 newWidth, u32 newHeight) = 0;

	virtual void Update() = 0;
	virtual void Draw() = 0;
};
