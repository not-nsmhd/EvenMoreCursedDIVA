#pragma once
#include <string>
#include <deque>
#include "game.h"
#include "gamescore.h"
#include "gfx/sprite_sheet.h"
#include "gfx/sprite_renderer.h"
#include "audio/audio.h"
#include "chart.h"
#include "note.h"

namespace MainGame
{
	class MainGameState : public GameState
	{
	public:
		MainGameState();

		static MainGameState* GetInstance();

		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void OnResize(u32 newWidth, u32 newHeight);
		void Update();
		void Draw();

	private:
		static MainGameState* instance;

		GFX::SpriteRenderer spriteRenderer;
		GFX::SpriteSheet iconSet;

		Audio::SoundEffect hitSE = {};

		const vec2 noteArea_BaseSize = { 1280.0f, 720.0f };
		vec2 noteArea_ScaleFactor = {};

		float elapsedTime = 0.0f;
		bool manualUpdate = false;
		bool autoPlay = false;

		Chart songChart;
		size_t chartNoteOffset = 0;

		std::deque<Note> activeNotes = {};

		GameScore gameScore;

		char debugStateString[1024] = {};
		std::string noteValu;
		vec2 noteHitPos = {};
		bool noteWrong = false;

		void gameStep();
		void inputNoteHit(NoteShape shape, bool secondary, bool release);

		void handleNoteInput();
		void handleDebugInput();

		void updateDebug();
		void drawDebug();
	};
}
