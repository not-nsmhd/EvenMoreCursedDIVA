#pragma once
#include <string>
#include <deque>
#include "game.h"
#include "gfx/sprite_sheet.h"
#include "gfx/sprite_renderer.h"
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

		const vec2 noteArea_BaseSize = { 1280.0f, 720.0f };
		vec2 noteArea_ScaleFactor = {};

		float elapsedTime = 0.0f;
		bool manualUpdate = false;

		Chart songChart;
		size_t chartNoteOffset = 0;

		std::deque<Note> activeNotes = {};

		char debugStateString[1024] = {};
		char noteHitTime[32] = {};
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
