#pragma once
#include "../gfx/sprite_sheet.h"
#include "../gfx/sprite_renderer.h"
#include "chart.h"
#include "gamescore.h"
#include <glm/vec2.hpp>

namespace MainGame
{
	using glm::vec2;

	#if 0
	class Note
	{
	public:
		Note();
		Note(float time, NoteShape shape, vec2 position, vec2 scaleFactor);
		Note(float barTime, const ChartNote* chartNote, vec2 scaleFactor);

		void SetResources(GFX::SpriteSheet* iconSet, GFX::Sprite** iconSprites, GFX::Sprite** targetSprites, GFX::Sprite* targetHandSprite);

		vec2 GetTargetPosition();
		NoteShape GetShape();

		bool HasExpired();
		bool HasBeenHit();
		bool HasBeenWrongHit();

		float GetRemainingTime();
		float GetRemainingTimeOnHit();

		void Update(float deltaTime_ms);
		void Draw(SpriteRenderer& sprRenderer);

		void SendInput(NoteShape shape, bool secondary, bool holding);
		HitValuation GetHitValuation();
	private:
		NoteShape shape;

		float noteTime = 60.0f;
		float elapsedTime = 0.0f;
		float remainingTime = 0.0f;

		float elapsedTime_normalized = 0.0f;
		float remainingTime_normalized = 0.0f;

		float remainingTimeOnHit = 0.0f;

		bool expired = false;
		bool hasBeenHit = false;
		HitValuation valuation = HitValuation::NONE;
		bool wrongHit = false;

		vec2 targetPosition = { 0.0f, 0.0f };
		vec2 iconPosition = { FLT_MIN, FLT_MIN };
		vec2 scaleFactor = {};

		float angle = 0.0f;
		float distance = 0.0f;
		float frequency = 0.0f;
		float amplitude = 0.0f;

		GFX::SpriteSheet* iconSet;
		GFX::Sprite* targetSprite;
		GFX::Sprite* iconSprite;
		GFX::Sprite* targetHandSprite;
	};
	#endif

	enum class GameNoteState
	{
		NONE = -1,

		ACTIVE,
		EXPIRED,

		HIT,
		HIT_SECONDARY,

		HELD,
		RELEASED
	};

	const u32 TRAIL_POINTS_RESOLUTION = 48;
	const float MAX_NOTE_DURATION_NORMALIZED = 1.25f;
	const u32 TRAIL_POINTS_COUNT = static_cast<u32>(TRAIL_POINTS_RESOLUTION * MAX_NOTE_DURATION_NORMALIZED);

	struct GameNote
	{
		ChartNote* noteStats = nullptr;
		GameNoteState state = GameNoteState::NONE;
		GameNoteState secondaryState = GameNoteState::NONE;
		int index = -1;

		float flyTime_seconds = 0.0f;
		float elapsedTime_seconds = 0.0f;

		float elapsedTimeOnHit_seconds = 0.0f;

		glm::vec2 trailPoints[TRAIL_POINTS_COUNT] = {};
		float trailScrollOffset = 0.0f;
	};
}
