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

		if (Expired || ShouldBeRemoved || HasBeenHit) { return; }

		if ((GetRemainingTime() * 1000.0) <= HitThresholds::ThresholdMiss)
		{
			Expiring = true;
		}

		if (GetRemainingTime() <= NoteRemoveTimeThreshold)
		{
			ShouldBeRemoved = true;
			return;
		}

		IconPosition = MathExtensions::GetSinePoint(1.0 - GetNormalizedElapsedTime(), TargetPosition, EntryAngle, Frequency, Amplitude, Distance);
	}

	void GameNote::Draw(f64 deltaTime_ms)
	{
		if (Expired || ShouldBeRemoved || HasBeenHit) { return; }

		auto& sprRenderer = MainGameContext->SpriteRenderer;
		auto& iconSet = MainGameContext->IconSetSprites;

		const Sprite* targetSprite = iconSet.NoteTargets[static_cast<size_t>(Shape)];
		const Sprite* iconSprite = iconSet.NoteIcons[static_cast<size_t>(Shape)];

		sprRenderer->SpriteSheet().PushSprite(iconSet.SpriteSheet, *targetSprite, TargetPosition, vec2(1.0f), DefaultColors::White);

		sprRenderer->SetSpriteRotation(GetNormalizedElapsedTime() * MathExtensions::TwoPi);
		sprRenderer->SpriteSheet().PushSprite(iconSet.SpriteSheet, *iconSet.NoteTargetHand, TargetPosition, vec2(1.0f), DefaultColors::White);

		sprRenderer->SpriteSheet().PushSprite(iconSet.SpriteSheet, *iconSprite, IconPosition, vec2(1.0f), DefaultColors::White);
	}

	bool GameNote::Evaluate(NoteShape shape, bool ignoreWrong)
	{
		f64 remainingTime_ms = GetRemainingTime() * 1000.0;

		if (remainingTime_ms > HitThresholds::ThresholdStart) { return false; }
		bool shapeMatches = Shape == shape;

		if (!shapeMatches && ignoreWrong) { return false; }

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
