#include "GameNote.h"
#include "Common/MathExt.h"

namespace DIVA::MainGame
{
	using namespace Starshine;
	using namespace Starshine::GFX::Render2D;

	constexpr f64 NoteRemoveTimeThreshold{ -1.0 };

	f64 GameNote::GetRemainingTime() const
	{
		return FlyTime - ElapsedTime;
	}

	f64 GameNote::GetNormalizedElapsedTime() const
	{
		return MathExtensions::ConvertRange<f64>(0.0f, FlyTime, 0.0f, 1.0f, ElapsedTime);
	}

	bool GameNote::HasBeenEvaluated() const
	{
		return HitEvaluation != HitEvaluation::None;
	}

	void GameNote::Update(f64 deltaTime_ms)
	{
		ElapsedTime += deltaTime_ms / 1000.0;

		if (Expired || ShouldBeRemoved) { return; }

		if ((GetRemainingTime() * 1000.0) <= HitThresholds::ThresholdMiss && !HasBeenHit)
		{
			Expiring = true;
		}

		if (GetRemainingTime() <= NoteRemoveTimeThreshold)
		{
			if (NextNote != nullptr)
			{
				ShouldBeRemoved = NextNote->ShouldBeRemoved;
			}
			else
			{
				ShouldBeRemoved = true;
			}
		}

		if (ShouldBeRemoved) { return; }

		IconPosition = MathExtensions::GetSinePoint(1.0 - GetNormalizedElapsedTime(), TargetPosition, EntryAngle, Frequency, Amplitude, Distance);
	}

	void GameNote::Draw(f64 deltaTime_ms)
	{
		if (Expired || ShouldBeRemoved || ElapsedTime < 0.0f) { return; }
		if (HasBeenHit && Type != NoteType::HoldStart) { return; }

		if (HasBeenHit && NextNote != nullptr)
		{
			if (NextNote->HasBeenHit) { return; }
		}

		auto& sprRenderer = MainGameContext->SpriteRenderer;
		auto& iconSet = MainGameContext->IconSetSprites;

		const Sprite* targetSprite = nullptr;
		const Sprite* targetHandSprite = nullptr;
		const Sprite* iconSprite = nullptr;

		switch (Type)
		{
		case NoteType::Normal:
		default:
			targetSprite = iconSet.NoteTargets[static_cast<size_t>(Shape)];
			iconSprite = iconSet.NoteIcons[static_cast<size_t>(Shape)];
			targetHandSprite = iconSet.NoteTargetHand;
			break;
		case NoteType::Double:
			targetSprite = iconSet.DoubleNoteTargets[static_cast<size_t>(Shape)];
			iconSprite = iconSet.DoubleNoteIcons[static_cast<size_t>(Shape)];
			targetHandSprite = iconSet.DoubleNoteTargetHands[static_cast<size_t>(Shape)];
			break;
		case NoteType::HoldStart:
		case NoteType::HoldEnd:
			targetSprite = iconSet.HoldNoteTargets[static_cast<size_t>(Shape)];
			iconSprite = iconSet.HoldNoteIcons[static_cast<size_t>(Shape)];
			targetHandSprite = iconSet.NoteTargetHand;
			break;
		}

		if (Type != NoteType::HoldEnd)
		{
			sprRenderer->SpriteSheet().PushSprite(iconSet.SpriteSheet, *targetSprite, TargetPosition, vec2(1.0f), DefaultColors::White);
		}

		sprRenderer->SetSpriteRotation(!HasBeenHit ? (GetNormalizedElapsedTime() * MathExtensions::TwoPi) : 0.0f);
		sprRenderer->SpriteSheet().PushSprite(iconSet.SpriteSheet, *targetHandSprite, TargetPosition, vec2(1.0f), DefaultColors::White);

		if (!HasBeenHit)
		{
			sprRenderer->SpriteSheet().PushSprite(iconSet.SpriteSheet, *iconSprite, IconPosition, vec2(1.0f), DefaultColors::White);
		}
	}

	bool GameNote::Evaluate(NoteShape shape)
	{
		f64 remainingTime_ms = GetRemainingTime() * 1000.0;
		bool shapeMatches = Shape == shape;

		if (remainingTime_ms > HitThresholds::ThresholdStart)
		{
			if (Type == NoteType::HoldEnd && shapeMatches)
			{
				HasBeenHit = true;
				HitEvaluation = HitEvaluation::Miss;
				return true;
			}

			return false;
		}
		
		if (Type == NoteType::Double && (!DoubleTap.Primary || !DoubleTap.Alternative))
		{
			bool holdTrans = false;
			if (DoubleTap.Primary && Hold.AlternativeHeld) { holdTrans = true; }
			else if (DoubleTap.Alternative && Hold.PrimaryHeld) { holdTrans = true; }

			if (!holdTrans) { return false; }
			else { DoubleTap.GiveBonus = false; }
		}

		if (MathExtensions::IsInRange(-HitThresholds::CoolThreshold, HitThresholds::CoolThreshold, remainingTime_ms))
		{
			HitEvaluation = HitEvaluation::Cool;
			HasBeenHit = true;
			HitWrong = !shapeMatches;
			return true;
		}
		else if (MathExtensions::IsInRange(-HitThresholds::GoodThreshold, HitThresholds::GoodThreshold, remainingTime_ms))
		{
			HitEvaluation = HitEvaluation::Good;
			HasBeenHit = true;
			HitWrong = !shapeMatches;
			return true;
		}
		else if (MathExtensions::IsInRange(-HitThresholds::SafeThreshold, HitThresholds::SafeThreshold, remainingTime_ms))
		{
			HitEvaluation = HitEvaluation::Safe;
			HasBeenHit = true;
			HitWrong = !shapeMatches;
			return true;
		}
		else if (MathExtensions::IsInRange(-HitThresholds::BadThreshold, HitThresholds::BadThreshold, remainingTime_ms))
		{
			HitEvaluation = HitEvaluation::Bad;
			HasBeenHit = true;
			HitWrong = !shapeMatches;
			return true;
		}
		else
		{
			HitEvaluation = HitEvaluation::Miss;
			return true;
		}

		return false;
	}
}
