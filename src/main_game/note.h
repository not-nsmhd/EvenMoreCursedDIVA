#pragma once
#include "../gfx/sprite_sheet.h"
#include "../gfx/sprite_renderer.h"
#include "chart.h"
#include <glm/vec2.hpp>

namespace MainGame
{
	using glm::vec2;

	class Note
	{
	public:
		Note();
		Note(const GFX::SpriteSheet* iconSet, float time, NoteShape shape, vec2 position, vec2 scaleFactor);
		Note(const GFX::SpriteSheet* iconSet, float barTime, const ChartNote* chartNote, vec2 scaleFactor);

		vec2 GetTargetPosition();

		bool HasExpired();
		bool HasBeenHit();
		bool HasBeenWrongHit();
		float GetRemainingTimeOnHit();

		void Update(float deltaTime_ms);
		void Draw(SpriteRenderer& sprRenderer);

		void SendInput(NoteShape shape, bool secondary, bool holding);
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
		bool wrongHit = false;

		vec2 targetPosition = { 0.0f, 0.0f };
		vec2 iconPosition = { FLT_MIN, FLT_MIN };
		vec2 scaleFactor = {};

		float angle = 0.0f;
		float distance = 0.0f;
		float frequency = 0.0f;
		float amplitude = 0.0f;

		const GFX::SpriteSheet* iconSet;
		GFX::Sprite targetSprite;
		GFX::Sprite iconSprite;
		GFX::Sprite targetHandSprite;
	};
}
