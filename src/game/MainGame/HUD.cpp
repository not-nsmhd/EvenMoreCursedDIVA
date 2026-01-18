#include "hud.h"
#include "common/color.h"
#include <Common/MathExt.h>
#include "gfx/Render2D/SpriteSheet.h"
#include "gfx/Render2D/SpriteRenderer.h"

namespace DIVA::MainGame
{
	using namespace Starshine;
	using namespace Starshine::GFX::Render2D;
	using std::string_view;

	struct HUD::Impl
	{
		MainGame::MainGameContext& mainGameContext;

		Font* debugFont = nullptr;

		struct SpriteCache
		{
			SpriteSheet hudSprites;

			const Sprite* HitEvaluations[EnumCount<HitEvaluation>()]{};
			const Sprite* HitEvaluations_Wrong[EnumCount<HitEvaluation>()]{};

			const Sprite* ScoreNumbers[10]{};
			const Sprite* ComboNumbers[10]{};

			const Sprite* ScoreBonusNumbers[10]{};
			const Sprite* ScoreBonus_Plus{};
		} spriteCache;

		struct ComboDisplayData
		{
			vec2 Position{};
			HitEvaluation HitEvaluation{};
			bool IsWrong{};
			u32 Combo{};

			float ElapsedDisplayTime{};
		} ComboDisplayState;

		struct ScoreBonusDisplayData
		{
			vec2 Position{};
			u32 Value{};
			bool Held{};

			float ElapsedDisplayTime{};
		} ScoreBonusDisplay;

		struct ScoreDisplayData
		{
			vec2 Position{ 1250.0f, 30.0f };
			u32 DisplayValue{};
			float IncrementSpeed{ 0.02f };
		} ScoreDisplay;

		Impl(MainGame::MainGameContext& context) : mainGameContext{ context }
		{
		}

		void Initialize()
		{
		}

		void Reset()
		{
			ComboDisplayState.HitEvaluation = HitEvaluation::None;
			ScoreBonusDisplay.Value = 0;
			ScoreDisplay.DisplayValue = 0;
		}

		bool LoadSprites(GFX::SpritePacker& sprPacker)
		{
			sprPacker.Clear();
			sprPacker.Initialize();

			sprPacker.AddFromDirectory("diva/sprites/mg_hud");
			sprPacker.Pack();

			spriteCache.hudSprites.CreateFromSpritePacker(sprPacker);
			sprPacker.Clear();

			auto fetchHitValueSprite = [&](HitEvaluation valu, std::string_view name, const Sprite* spriteArray[])
			{
				spriteArray[static_cast<size_t>(valu)] = &spriteCache.hudSprites.GetSprite(name);
			};

			auto fetchSprite = [&](std::string_view name)
			{
				return &spriteCache.hudSprites.GetSprite(name);
			};

			fetchHitValueSprite(HitEvaluation::Cool, "HitValu_Cool", spriteCache.HitEvaluations);
			fetchHitValueSprite(HitEvaluation::Good, "HitValu_Good", spriteCache.HitEvaluations);
			fetchHitValueSprite(HitEvaluation::Safe, "HitValu_Safe", spriteCache.HitEvaluations);
			fetchHitValueSprite(HitEvaluation::Bad, "HitValu_Bad", spriteCache.HitEvaluations);
			fetchHitValueSprite(HitEvaluation::Miss, "HitValu_Miss", spriteCache.HitEvaluations);

			fetchHitValueSprite(HitEvaluation::Cool, "HitValu_Cool_Wrong", spriteCache.HitEvaluations_Wrong);
			fetchHitValueSprite(HitEvaluation::Good, "HitValu_Good_Wrong", spriteCache.HitEvaluations_Wrong);
			fetchHitValueSprite(HitEvaluation::Safe, "HitValu_Safe_Wrong", spriteCache.HitEvaluations_Wrong);
			fetchHitValueSprite(HitEvaluation::Bad, "HitValu_Miss", spriteCache.HitEvaluations_Wrong);
			fetchHitValueSprite(HitEvaluation::Miss, "HitValu_Miss", spriteCache.HitEvaluations_Wrong);

			spriteCache.ScoreNumbers[0] = fetchSprite("Score_0");
			spriteCache.ScoreNumbers[1] = fetchSprite("Score_1");
			spriteCache.ScoreNumbers[2] = fetchSprite("Score_2");
			spriteCache.ScoreNumbers[3] = fetchSprite("Score_3");
			spriteCache.ScoreNumbers[4] = fetchSprite("Score_4");
			spriteCache.ScoreNumbers[5] = fetchSprite("Score_5");
			spriteCache.ScoreNumbers[6] = fetchSprite("Score_6");
			spriteCache.ScoreNumbers[7] = fetchSprite("Score_7");
			spriteCache.ScoreNumbers[8] = fetchSprite("Score_8");
			spriteCache.ScoreNumbers[9] = fetchSprite("Score_9");

			spriteCache.ComboNumbers[0] = fetchSprite("Combo_0");
			spriteCache.ComboNumbers[1] = fetchSprite("Combo_1");
			spriteCache.ComboNumbers[2] = fetchSprite("Combo_2");
			spriteCache.ComboNumbers[3] = fetchSprite("Combo_3");
			spriteCache.ComboNumbers[4] = fetchSprite("Combo_4");
			spriteCache.ComboNumbers[5] = fetchSprite("Combo_5");
			spriteCache.ComboNumbers[6] = fetchSprite("Combo_6");
			spriteCache.ComboNumbers[7] = fetchSprite("Combo_7");
			spriteCache.ComboNumbers[8] = fetchSprite("Combo_8");
			spriteCache.ComboNumbers[9] = fetchSprite("Combo_9");

			spriteCache.ScoreBonusNumbers[0] = fetchSprite("ScoreBonus_0");
			spriteCache.ScoreBonusNumbers[1] = fetchSprite("ScoreBonus_1");
			spriteCache.ScoreBonusNumbers[2] = fetchSprite("ScoreBonus_2");
			spriteCache.ScoreBonusNumbers[3] = fetchSprite("ScoreBonus_3");
			spriteCache.ScoreBonusNumbers[4] = fetchSprite("ScoreBonus_4");
			spriteCache.ScoreBonusNumbers[5] = fetchSprite("ScoreBonus_5");
			spriteCache.ScoreBonusNumbers[6] = fetchSprite("ScoreBonus_6");
			spriteCache.ScoreBonusNumbers[7] = fetchSprite("ScoreBonus_7");
			spriteCache.ScoreBonusNumbers[8] = fetchSprite("ScoreBonus_8");
			spriteCache.ScoreBonusNumbers[9] = fetchSprite("ScoreBonus_9");
			spriteCache.ScoreBonus_Plus = fetchSprite("ScoreBonus_Plus");

			return true;
		}

		void UpdateScoreDisplay(float deltaTime_ms)
		{
			i32 scoreDiff = mainGameContext.Score.Score - ScoreDisplay.DisplayValue;

			if (scoreDiff < 61)
			{
				if (scoreDiff < 21)
				{
					ScoreDisplay.DisplayValue = mainGameContext.Score.Score;
				}
				else
				{
					ScoreDisplay.DisplayValue += 7;
				}
			}
			else
			{
				float incrementSpeed = (deltaTime_ms / 16.6667f) * ScoreDisplay.IncrementSpeed;
				ScoreDisplay.DisplayValue += static_cast<u32>((incrementSpeed * static_cast<float>((scoreDiff * 10))));
			}
		}

		void UpdateComboDisplay(float deltaTime_ms)
		{
			if (ComboDisplayState.ElapsedDisplayTime <= 2.0f)
			{
				ComboDisplayState.ElapsedDisplayTime += deltaTime_ms / 1000.0f;
			}
		}

		void UpdateScoreBonusDisplay(float deltaTime_ms)
		{
			if (ScoreBonusDisplay.ElapsedDisplayTime <= 1.0f && !ScoreBonusDisplay.Held)
			{
				ScoreBonusDisplay.ElapsedDisplayTime += deltaTime_ms / 1000.0f;
			}
		}
		
		// NOTE: Value text is displayed from right to left
		float DrawSpriteNumericValue(u32 value, const Sprite* spriteArray[], vec2& position, vec2& scale, float spacing, int length = -1)
		{
			SpriteSheetRenderer& sprRenderer = mainGameContext.SpriteRenderer->SpriteSheet();
			vec2 displayOffset{ 0.0f, 0.0f };

			int realLength = (length == -1) ? 10 : length;

			u32 remainingNumbers = value;
			for (int i = 0; i < realLength; i++)
			{
				if (remainingNumbers == 0 && i > 0 && length == -1) { break; }

				int sprIndex = remainingNumbers % 10;
				const Sprite* numSprite = spriteArray[sprIndex];

				sprRenderer.PushSprite(spriteCache.hudSprites, *numSprite, position + displayOffset, scale, DefaultColors::White);

				displayOffset.x -= spacing * scale.x;
				remainingNumbers /= 10;
			}

			return displayOffset.x;
		}

		float MeasureSpriteNumericValue(u32 value, const Sprite* spriteArray[], float spacing, int length = -1)
		{
			float displayOffset{ 0.0f };

			int realLength = (length == -1) ? 10 : length;

			u32 remainingNumbers = value;
			for (int i = 0; i < realLength; i++)
			{
				if (remainingNumbers == 0 && i > 0 && length == -1) { return displayOffset; }

				int sprIndex = remainingNumbers % 10;
				const Sprite* numSprite = spriteArray[sprIndex];

				displayOffset += spacing;
				remainingNumbers /= 10;
			}

			return displayOffset;
		}

		void DrawScoreDisplay()
		{
			DrawSpriteNumericValue(ScoreDisplay.DisplayValue, spriteCache.ScoreNumbers, ScoreDisplay.Position, vec2(1.0f), 27.0f, -1);
		}

		void DrawComboDisplay(float deltaTime_ms)
		{
			if (ComboDisplayState.HitEvaluation != HitEvaluation::None && ComboDisplayState.ElapsedDisplayTime <= 2.0f)
			{
				SpriteSheetRenderer& sprRenderer = mainGameContext.SpriteRenderer->SpriteSheet();

				size_t valuIndex = static_cast<size_t>(ComboDisplayState.HitEvaluation);

				const Sprite* valuSprite = ComboDisplayState.IsWrong ? spriteCache.HitEvaluations_Wrong[valuIndex] : 
					spriteCache.HitEvaluations[valuIndex];

				vec2 valuTextPos = { ComboDisplayState.Position.x, ComboDisplayState.Position.y - 35.0f };

				if (ComboDisplayState.Combo <= 1)
				{
					sprRenderer.PushSprite(spriteCache.hudSprites, *valuSprite, valuTextPos, vec2(1.0f), DefaultColors::White);
				}
				else
				{
					constexpr float valuComboSpacing = 27.0f;

					float comboTextWidth = MeasureSpriteNumericValue(ComboDisplayState.Combo, spriteCache.ComboNumbers, 25.0f);

					vec2 comboTextPos = { valuTextPos.x + valuComboSpacing + (comboTextWidth / 2.0f), valuTextPos.y };
					valuTextPos.x -= (comboTextWidth / 2.0f) + valuComboSpacing;

					sprRenderer.PushSprite(spriteCache.hudSprites, *valuSprite, valuTextPos, vec2(1.0f), DefaultColors::White);
					DrawSpriteNumericValue(ComboDisplayState.Combo, spriteCache.ComboNumbers, comboTextPos, vec2(1.0f), 25.0f);
				}
			}
		}

		void DrawScoreBonusDisplay(float deltaTime_ms)
		{
			if (ScoreBonusDisplay.Value > 0 && ScoreBonusDisplay.ElapsedDisplayTime <= 1.0f)
			{
				SpriteSheetRenderer& sprRenderer = mainGameContext.SpriteRenderer->SpriteSheet();

				float textWidth = MeasureSpriteNumericValue(ScoreBonusDisplay.Value * 10, spriteCache.ScoreBonusNumbers, 22.0f);
				float plusWidth = spriteCache.ScoreBonus_Plus->SourceRectangle.Width;

				float textPosY = MathExtensions::ConvertRange(0.0f, 1.0f, 0.0f, -20.0f, ScoreBonusDisplay.ElapsedDisplayTime);

				vec2 textPos = { ScoreBonusDisplay.Position.x, ScoreBonusDisplay.Position.y - 70.0f };
				DrawSpriteNumericValue(ScoreBonusDisplay.Value, spriteCache.ScoreBonusNumbers, textPos, vec2(1.0f), 22.0f);

				//textPos.x -= (textWidth / 2.0f) - (plusWidth / 2.0f);
				//sprRenderer.PushSprite(spriteCache.hudSprites, *spriteCache.ScoreBonus_Plus, textPos, vec2(1.0f), DefaultColors::White);
			}
		}

		void SetScoreBonusDisplayState(u32 value, vec2& position)
		{
			ScoreBonusDisplay.Position = position;
			ScoreBonusDisplay.Value = value;
			ScoreBonusDisplay.ElapsedDisplayTime = 0.0f;
		}

		void HoldScoreBonusDisplay()
		{
			ScoreBonusDisplay.Held = true;
		}

		void ReleaseScoreBonusDisplay(bool drop)
		{
			if (drop)
			{
				ScoreBonusDisplay.ElapsedDisplayTime = 2.0f;
			}
			ScoreBonusDisplay.Held = false;
		}
	};

	void HUD::Initialize()
	{
		impl = new Impl(mainGameContext);
		impl->Initialize();
	}

	void HUD::Reset()
	{
		impl->Reset();
	}

	bool HUD::LoadSprites(Starshine::GFX::SpritePacker& sprPacker)
	{
		return impl->LoadSprites(sprPacker);
	}

	void HUD::Destroy()
	{
		delete impl;
	}

	void HUD::Update(float deltaTime_ms)
	{
		impl->UpdateScoreDisplay(deltaTime_ms);
		impl->UpdateComboDisplay(deltaTime_ms);
		impl->UpdateScoreBonusDisplay(deltaTime_ms);
	}

	void HUD::Draw(float deltaTime_ms)
	{
		impl->DrawComboDisplay(deltaTime_ms);
		impl->DrawScoreBonusDisplay(deltaTime_ms);
		impl->DrawScoreDisplay();
	}

	void HUD::SetComboDisplayState(HitEvaluation hitEvaluation, u32 combo, bool wrong, vec2& position)
	{
		if (hitEvaluation != HitEvaluation::None)
		{
			impl->ComboDisplayState.Position = position;
			impl->ComboDisplayState.HitEvaluation = hitEvaluation;
			impl->ComboDisplayState.IsWrong = wrong;
			impl->ComboDisplayState.Combo = combo;

			impl->ComboDisplayState.ElapsedDisplayTime = 0.0f;
		}
	}

	void HUD::SetScoreBonusDisplayState(u32 value, vec2& position)
	{
		if (value > 0)
		{
			impl->SetScoreBonusDisplayState(value, position);
		}
	}

	void HUD::HoldScoreBonus()
	{
		impl->HoldScoreBonusDisplay();
	}

	void HUD::ReleaseScoreBonus(bool drop)
	{
		impl->ReleaseScoreBonusDisplay(drop);
	}
}
