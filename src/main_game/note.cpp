#include "note.h"
#include "../common/math_ext.h"
#include <string>

using std::string;
using namespace Common;

namespace MainGame
{
	#if 0
	Note::Note() : iconSet(nullptr)
	{
	}

	Note::Note(float time, NoteShape shape, vec2 position, vec2 scaleFactor) : iconSet(iconSet), shape(shape)
	{
		noteTime = time;
		targetPosition = position * scaleFactor;
		this->scaleFactor = scaleFactor;
	}
	
	Note::Note(float barTime, const ChartNote* chartNote, vec2 scaleFactor) : iconSet(iconSet)
	{
		shape = chartNote->Shape;
		noteTime = barTime;
		targetPosition = chartNote->Position * scaleFactor;
		angle = chartNote->Angle;
		distance = chartNote->Distance;
		frequency= chartNote->Frequency;
		amplitude = chartNote->Amplitude;
		this->scaleFactor = scaleFactor;
	}
	
	void Note::SetResources(GFX::SpriteSheet* iconSet, GFX::Sprite** iconSprites, GFX::Sprite** targetSprites, GFX::Sprite* targetHandSprite)
	{
		if (shape != NoteShape::NOTE_NONE)
		{
			this->iconSet = iconSet;

			targetSprite = targetSprites[static_cast<int>(shape)];
			iconSprite = iconSprites[static_cast<int>(shape)];
			this->targetHandSprite = targetHandSprite;
		}
	}
	
	vec2 Note::GetTargetPosition()
	{
		return targetPosition;
	}
	
	NoteShape Note::GetShape()
	{
		return shape;
	}
	
	bool Note::HasExpired()
	{
		return expired;
	}
	
	bool Note::HasBeenHit()
	{
		return hasBeenHit;
	}
	
	bool Note::HasBeenWrongHit()
	{
		return wrongHit;
	}
	
	float Note::GetRemainingTime()
	{
		return remainingTime;
	}
	
	float Note::GetRemainingTimeOnHit()
	{
		return remainingTimeOnHit;
	}
	
	void Note::Update(float deltaTime_ms)
	{
		if (hasBeenHit || wrongHit || expired)
		{
			return;
		}

		elapsedTime += deltaTime_ms / 1000.0f;
		remainingTime = noteTime - elapsedTime;

		elapsedTime_normalized = MathExtensions::ConvertRange(0.0f, noteTime, 0.0f, 1.0f, elapsedTime);
		remainingTime_normalized = 1.0f - elapsedTime_normalized;

		if (remainingTime <= -0.333f)
		{
			valuation = HitValuation::MISS;
			expired = true;
		}

		// -------------

		iconPosition = MathExtensions::GetSinePoint(remainingTime_normalized, targetPosition, angle, frequency, amplitude, distance * scaleFactor.y);
	}
	
	void Note::Draw(SpriteRenderer& sprRenderer)
	{
		if (shape != NoteShape::NOTE_NONE && !hasBeenHit && !wrongHit && !expired)
		{
			sprRenderer.SetSpritePosition(targetPosition);
			sprRenderer.SetSpriteColor(DefaultColors::White);
			iconSet->PushSprite(sprRenderer, *targetSprite, scaleFactor);

			sprRenderer.SetSpritePosition(targetPosition);
			sprRenderer.SetSpriteColor(DefaultColors::White);
			sprRenderer.SetSpriteRotation(elapsedTime_normalized * MathExtensions::MATH_EXT_2PI);
			iconSet->PushSprite(sprRenderer, *targetHandSprite, scaleFactor);

			sprRenderer.SetSpritePosition(iconPosition);
			sprRenderer.SetSpriteColor(DefaultColors::White);
			iconSet->PushSprite(sprRenderer, *iconSprite, scaleFactor);
		}
	}
	
	void Note::SendInput(NoteShape shape, bool secondary, bool holding)
	{
		if (remainingTime > 0.13f | hasBeenHit)
		{
			return;
		}

		float hitTime_ms = remainingTime * 1000.0f;
		bool shapeMatches = this->shape == shape;

		wrongHit = !shapeMatches;
		remainingTimeOnHit = remainingTime;

		if (MathExtensions::IsInRange(-30.0f, 30.0f, hitTime_ms))
		{
			valuation = HitValuation::COOL;
		}
		else if (MathExtensions::IsInRange(-70.0f, 70.0f, hitTime_ms))
		{
			valuation = HitValuation::GOOD;
		}
		else if (MathExtensions::IsInRange(-100.0f, 100.0f, hitTime_ms))
		{
			valuation = HitValuation::SAFE;
		}
		else
		{
			valuation = HitValuation::BAD;
		}

		hasBeenHit = true;
	}
	
	HitValuation Note::GetHitValuation()
	{
		return valuation;
	}
	#endif


}