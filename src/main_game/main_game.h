#pragma once
#include <string>
#include <deque>
#include "game.h"
#include "gamescore.h"
#include "gfx/sprite_sheet.h"
#include "gfx/sprite_renderer.h"
#include "gfx/primitive_batch.h"
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

		// --- Cached sprites

		GFX::Sprite* cachedNoteTargetSprites[static_cast<int>(NoteShape::NOTE_SHAPE_COUNT)];
		GFX::Sprite* cachedNoteIconSprites[static_cast<int>(NoteShape::NOTE_SHAPE_COUNT)];

		GFX::Sprite* cachedDoubleTargetSprites[static_cast<int>(NoteShape::NOTE_SHAPE_COUNT)];
		GFX::Sprite* cachedDoubleIconSprites[static_cast<int>(NoteShape::NOTE_SHAPE_COUNT)];

		GFX::Sprite* noteTargetHandSprite;

		// --------------

		GFX::LowLevel::Texture* bgTexture = nullptr;
		GFX::LowLevel::Texture* trailTexture = nullptr;

		Audio::SoundEffect hitSE = {};
		Audio::SoundEffect hitSE_double = {};
		Audio::Music songMusic = {};
		bool musicStarted = false;

		const vec2 noteArea_BaseSize = { 1280.0f, 720.0f };
		vec2 noteArea_ScaleFactor = {};

		float elapsedTime = 0.0f;
		bool manualUpdate = false;
		bool autoPlay = false;

		bool paused = false;
		bool over = false;

		Chart songChart;
		size_t chartNoteOffset = 0;
		size_t chartEventOffset = 0;

		float currentNoteDuration_seconds = 1.0f;
		std::deque<GameNote> activeNotes = {};

		GameScore gameScore;

		char debugStateString[1024] = {};
		std::string noteValu;
		vec2 noteHitPos = {};
		bool noteWrong = false;

		void gameStep();
		void inputNoteHit(NoteShape shape, bool secondary, bool release);

		void drawNoteTrail(float t, GameNote* note);

		void handleNoteInput();
		void handleDebugInput();

		void updateDebug();
		void drawDebug();
	};
}
