#include "GameNote.h"
#include "Common/MathExt.h"

namespace DIVA::MainGame
{
	using namespace Starshine;
	using namespace Starshine::Rendering;
	using namespace Starshine::Rendering::Render2D;

	constexpr f64 NoteRemoveTimeThreshold{ -1.0 };
	constexpr f32 TrailMaxProgress{ 1.0f };

	f64 GameNote::GetRemainingTime() const
	{
		return FlyTime - ElapsedTime;
	}

	f64 GameNote::GetNormalizedElapsedTime() const
	{
		return ElapsedTime / FlyTime;
	}

	f64 GameNote::GetNormalizedRemainingTime() const
	{
		return 1.0 - GetNormalizedElapsedTime();
	}

	bool GameNote::HasBeenEvaluated() const
	{
		return HitEvaluation != HitEvaluation::None;
	}

	void GameNote::UpdateTrail()
	{
		if (Expiring || Expired || (HasBeenHit && Type != NoteType::HoldStart)) { return; }

		if (Type == NoteType::HoldStart && NextNote != nullptr)
		{
			if (NextNote->Expired || NextNote->HasBeenHit)
			{
				Trail.Start = 0.0f;
				Trail.End = 0.0f;
			}
			else
			{
				Trail.Start = MathExtensions::Clamp<f32>(HasBeenHit ? 0.0f : GetNormalizedRemainingTime(), 0.0f, TrailMaxProgress);
				Trail.End = MathExtensions::Min<f32>(TrailMaxProgress, NextNote->GetNormalizedRemainingTime());
			}
			Trail.Hold = true;
		}

		if (!HasBeenHit)
		{
			if (Type == NoteType::Normal || Type == NoteType::Double)
			{
				static constexpr f32 lengthFactor = 2.15f;

				f32 trailPixelLength = (Distance / 1000.0f) * (120.0f / static_cast<f32>(FlyTime) * lengthFactor);
				f32 normalizedLength = trailPixelLength / Distance;

				Trail.Start = MathExtensions::Clamp<f32>(GetNormalizedRemainingTime(), 0.0f, TrailMaxProgress);
				Trail.End = Trail.Start + normalizedLength;

				//DrawTrail();
			}
		}
	}

	void GameNote::DrawTrail()
	{
		if (Expiring || Expired || (HasBeenHit && Type != NoteType::HoldStart)) { return; }

		auto& sprRenderer = MainGameContext->SpriteRenderer;
		auto& iconSet = MainGameContext->IconSetSprites;

		static constexpr size_t trailSegmentCount = 48;
		static constexpr f32 trailSegmentStep = TrailMaxProgress / static_cast<f32>(trailSegmentCount);
		
		std::array<vec2, trailSegmentCount> trailSegments{};
		f32 segmentStep = (Trail.End - Trail.Start) / static_cast<f32>(trailSegmentCount);

		for (size_t i = 0; i < trailSegments.size(); i++)
		{
			trailSegments[i] = MathExtensions::GetSinePoint(MathExtensions::Min(Trail.End, Trail.Start + static_cast<f32>(i) * segmentStep),
				TargetPosition, EntryAngle, Frequency, Amplitude, Distance);
		}

		static constexpr size_t verticesPerSegment = 2;
		std::array<SpriteVertex, trailSegmentCount * verticesPerSegment> trailVertices{};

		const Sprite* trailSprite = Trail.Hold ? iconSet.HoldNoteTrails[static_cast<size_t>(Shape)] :
			(ActiveDuringChanceTime ? iconSet.Trail_CT : iconSet.Trail_Normal);

		const RectangleF& spriteRect = trailSprite->SourceRectangle;

		Texture* trailTexture = iconSet.SpriteSheet.GetTexture(trailSprite->TextureIndex);
		const f32 texWidth = trailTexture->GetWidth();
		const f32 texHeight = trailTexture->GetHeight();

		static constexpr std::array<u8, trailSegmentCount> trailAlphaValues
		{
			0, 56, 76, 90, 100, 108, 114, 119, 122, 125, 126, 127,
			127, 127, 126, 125, 124, 122, 120, 117, 114, 111, 108,
			105, 101, 98, 94, 90, 86, 82, 78, 74, 69, 65, 61, 56,
			52, 47, 43, 39, 34, 30, 25, 21, 17, 12, 0
		};

		static constexpr std::array<u8, trailSegmentCount> trailAlphaValues_ChanceTime
		{
			0, 25, 140, 180, 200, 216, 228, 237, 244, 249, 252, 254, 254,
			254, 252, 250, 247, 243, 239, 234, 228, 222, 216, 209, 202, 195,
			188, 180, 172, 164, 155, 147, 138, 130, 121, 112, 104, 95, 86,
			77, 68, 60, 51, 42, 34, 20, 0
		};

		static constexpr Color trailColors[EnumCount<NoteShape>()]
		{
			{ 237,  68,  78, 255 },
			{ 181, 255, 255, 255 },
			{ 255, 206, 255, 255 },
			{ 242, 255, 175, 255 },
			{ 255, 202, 0, 255 }
		};

		const Color trailColor = (Trail.Hold || ActiveDuringChanceTime) ? DefaultColors::White : trailColors[static_cast<size_t>(Shape)];

		const f32 segmentDistance = glm::distance(trailSegments[0], trailSegments[1]) / (spriteRect.Width * 0.05f);

		const auto getNormal = [](vec2 v) { return vec2(v.y, -v.x); };
		for (size_t i = 0, v = 0; i < trailSegments.size(); i++, v += 2)
		{
			const auto normal = (i < 1) ? glm::normalize(getNormal(trailSegments[i + 1] - trailSegments[i])) :
				(i >= trailSegmentCount - 1) ? glm::normalize(getNormal(trailSegments[i] - trailSegments[i - 1])) :
				glm::normalize(getNormal(trailSegments[i] - trailSegments[i - 1]) + getNormal(trailSegments[i + 1] - trailSegments[i]));

			const f32 thickness = (ActiveDuringChanceTime && !Trail.Hold) ? 0.7f : 0.5f;

			trailVertices[v + 0].Position = trailSegments[i] + normal * spriteRect.Height * thickness;
			trailVertices[v + 1].Position = trailSegments[i] - normal * spriteRect.Height * thickness;

			const u8 alpha = ActiveDuringChanceTime ? trailAlphaValues_ChanceTime[i] : trailAlphaValues[i];

			trailVertices[v + 0].Color = Trail.Hold ? DefaultColors::White : Color(trailColor.R, trailColor.G, trailColor.B, alpha);
			trailVertices[v + 1].Color = Trail.Hold ? DefaultColors::White : Color(trailColor.R, trailColor.G, trailColor.B, alpha);

			if (!Trail.Hold)
			{
				const f32 index = static_cast<f32>(i);
				trailVertices[v + 0].TexCoord = vec2(
					(spriteRect.X + index * segmentDistance + Trail.Scroll) / texWidth,
					spriteRect.Y / texHeight);

				trailVertices[v + 1].TexCoord = vec2(
					(spriteRect.X + (index + 1.0f) * segmentDistance + Trail.Scroll) / texWidth,
					(spriteRect.Y + spriteRect.Height) / texHeight);
			}
			else
			{
				trailVertices[v + 0].TexCoord = vec2(
					spriteRect.X / texWidth,
					spriteRect.Y / texHeight);

				trailVertices[v + 1].TexCoord = vec2(
					(spriteRect.X + spriteRect.Width) / texWidth,
					(spriteRect.Y + spriteRect.Height) / texHeight);
			}
		}

		sprRenderer->PushShape(trailVertices.data(), trailVertices.size(), PrimitiveType::TriangleStrip, trailTexture);
	}

	void GameNote::Update(f64 deltaTime_ms)
	{
		ElapsedTime += deltaTime_ms / 1000.0;

		if (ShouldBeRemoved) { return; }

		if ((GetRemainingTime() * 1000.0) <= HitThresholds::ThresholdMiss && !HasBeenHit)
		{
			Expiring = true;
			if (NextNote != nullptr) { NextNote->Expiring = true; }
		}

		if (GetRemainingTime() <= NoteRemoveTimeThreshold)
		{
			if (NextNote != nullptr)
			{
				if (Expiring) { ShouldBeRemoved = true; NextNote->ShouldBeRemoved = true; }
				else { ShouldBeRemoved = NextNote->ShouldBeRemoved; }
			}
			else
			{
				ShouldBeRemoved = true;
			}
		}

		IconPosition = MathExtensions::GetSinePoint(GetNormalizedRemainingTime(), TargetPosition, EntryAngle, Frequency, Amplitude, Distance);
		Trail.Scroll = std::fmodf(Trail.Scroll + 0.64f * (16.6667 / deltaTime_ms), Trail.ScrollResetThreshold);

		if (Expired || Expiring || ShouldBeRemoved) { return; }

		if (Type == NoteType::HoldStart)
		{
			if (HasBeenHit && (Hold.PrimaryHeld || Hold.AlternativeHeld))
			{
				if (!NextNote->HasBeenHit && !NextNote->Expiring)
				{
					i32 bonusMultiplier = HitWrong ? 0 : (HitEvaluation == HitEvaluation::Cool ? 20 : (HitEvaluation == HitEvaluation::Good ? 10 : 0));

					Hold.TimeSinceHoldStart += deltaTime_ms / 100.0;
					Hold.CurrentBonus = bonusMultiplier + static_cast<i32>(Hold.TimeSinceHoldStart) * bonusMultiplier + Hold.BonusBaseValue;
				}
			}
		}
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

		if (!HasBeenHit)
		{
			sprRenderer->SpriteSheet().PushSprite(iconSet.SpriteSheet, *targetSprite, TargetPosition, vec2(1.0f), DefaultColors::White);

			sprRenderer->SetSpriteRotation(!HasBeenHit ? (GetNormalizedElapsedTime() * MathExtensions::TwoPi) : 0.0f);
			sprRenderer->SpriteSheet().PushSprite(iconSet.SpriteSheet, *targetHandSprite, TargetPosition, vec2(1.0f), DefaultColors::White);
		}

		if (Type == NoteType::HoldStart && NextNote != nullptr)
		{
			//DrawTrail();

			if (HasBeenHit && NextNote->ElapsedTime < 0.0f)
			{
				sprRenderer->SpriteSheet().PushSprite(iconSet.SpriteSheet, *targetSprite, TargetPosition, vec2(1.0f), DefaultColors::White);
				sprRenderer->SetSpriteRotation(0.0f);
				sprRenderer->SpriteSheet().PushSprite(iconSet.SpriteSheet, *targetHandSprite, TargetPosition, vec2(1.0f), DefaultColors::White);
			}
		}

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
