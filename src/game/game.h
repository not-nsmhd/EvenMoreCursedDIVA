#pragma once
#include <string>
#include <SDL2/SDL.h>
#include "common/types.h"
#include "io/filesystem.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "gfx/lowlevel/backend.h"

using std::string;

using IO::FileSystem;
using Input::Keyboard;
using Input::Mouse;

using GFXBackend = GFX::LowLevel::Backend;
using GFXBackendType = GFX::LowLevel::BackendType;

class Game;
class GameState;

enum class GameStates
{
	UnregisteredState = -2,
	None = -1,

	MainGame,
	DevTest_UnicodeText,
	DevTest_Input,
	DevTest_Audio,
	Dev_StateSelector,

	Count
};

constexpr const char* GameStateNames[DIVA::EnumCount<GameStates>()] = 
{
	"Main Game",
	"[Dev] UTF-16 Text Test",
	"[Dev] Input Test",
	"[Dev] Audio Test",
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

	GFXBackendType gfxBackendType = GFXBackendType::None;
	bool fullscreen = false;

	/* State properties */

	bool firstFrame;
	bool running;
	SDL_Event sdlEvent;

	/* Timing */

	u64 ticks_lastFrame = 0;
	u64 ticks_now = 0;
	u64 ticks_delta = 0;
	double deltaTime_ms = 0.0;
	double actualFrameTime_ms = 0.0;

	/* Game state list */
	GameStates currentGameState = GameStates::None;
	GameState* stateList[static_cast<int>(GameStates::Count)] = {};

	/* Constructor and destructor */

	Game();
	~Game();
	
	/* Public functions */

	void Quit();

	bool Initialize();
	bool Loop();
	void SetState(GameStates state);

	bool IsActive();

	void GetVersionNumber(int* year, int* month);

	GFXBackend* GetGraphicsBackend();
private:
	bool initialized = false;
	bool active = true;

	/* File system */
	FileSystem* fileSystem;
	GFXBackend* graphicsBackend;

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

	GameState() { };

	virtual bool Initialize() = 0;
	virtual bool LoadContent() = 0;
	virtual void UnloadContent() = 0;
	virtual void Destroy() = 0;

	virtual void OnResize(u32 newWidth, u32 newHeight) = 0;

	virtual void Update() = 0;
	virtual void Draw() = 0;
};
