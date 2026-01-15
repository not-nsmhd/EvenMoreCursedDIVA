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

	constexpr EnumStringMappingTable<HitEvaluation> HitEvaluationNames
	{
		EnumStringMapping<HitEvaluation>
		{ HitEvaluation::None, "" },
		{ HitEvaluation::Cool, "Cool" },
		{ HitEvaluation::Good, "Good" },
		{ HitEvaluation::Safe, "Safe" },
		{ HitEvaluation::Bad, "Bad" },
		{ HitEvaluation::Miss, "Miss" }
	};

	constexpr Color HitEvaluationColors[EnumCount<HitEvaluation>()]
	{
		{   0,   0,   0,   0 },
		{ 255, 255,   0, 255 },
		{ 255, 255, 255, 255 },
		{   0, 255,   0, 255 },
		{ 128, 128, 255, 255 },
		{ 255,   0, 255, 255 }
	};

	struct HUD::Implementation
	{
		MainGame::Context& mainGameContext;

		Font* debugFont = nullptr;

		struct SpriteCache
		{
			SpriteSheet hudSprites;

			const Sprite* HitEvaluations[EnumCount<HitEvaluation>()]{};
			const Sprite* ScoreNumbers[10]{};
			const Sprite* ComboNumbers[10]{};
		} spriteCache;

		struct ComboDisplayData
		{
			vec2 Position{};
			HitEvaluation HitEvaluation{};
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
			vec2 Position{ 1260.0f, 40.0f };
			u32 DisplayValue{};
			float IncrementSpeed{ 0.02f };
		} ScoreDisplay;

		Implementation(MainGame::Context& context) : mainGameContext{ context }
		{
		}

		void Initialize()
		{
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

		void DrawScoreDisplay()
		{
			SpriteSheetRenderer& sprRenderer = mainGameContext.SpriteRenderer->SpriteSheet();
			vec2 displayOffset{ 0.0f, 0.0f };

			u32 remainingNumbers = ScoreDisplay.DisplayValue;
			for (int i = 0; i < 8; i++)
			{
				if (remainingNumbers == 0 && i > 0) { break; }

				int sprIndex = remainingNumbers % 10;
				const Sprite* numSprite = spriteCache.ScoreNumbers[sprIndex];

				sprRenderer.PushSprite(spriteCache.hudSprites, *numSprite, ScoreDisplay.Position + displayOffset, vec2(0.667f), DefaultColors::White);

				displayOffset.x -= 25.0f;
				remainingNumbers /= 10;
			}
		}

		void DrawComboDisplay(float deltaTime_ms)
		{
			if (ComboDisplayState.HitEvaluation != HitEvaluation::None && ComboDisplayState.ElapsedDisplayTime <= 2.0f)
			{
				const Font& debugFont = *mainGameContext.DebugFont;
				FontRenderer& fontRenderer = mainGameContext.SpriteRenderer->Font();

				string_view valuText = EnumToString<HitEvaluation>(HitEvaluationNames, ComboDisplayState.HitEvaluation);
				Color valuColor = HitEvaluationColors[static_cast<size_t>(ComboDisplayState.HitEvaluation)];
				Color comboColor = DefaultColors::White;

				vec2 valuTextPos = { ComboDisplayState.Position.x - 30.0f, ComboDisplayState.Position.y - 35.0f };

				if (ComboDisplayState.Combo <= 1)
				{
					fontRenderer.PushString(debugFont, valuText, valuTextPos, vec2(1.0f), valuColor);
				}
				else
				{
					char comboText[8] = {};
					size_t comboTextLength = snprintf(comboText, sizeof(comboText) - 1, "%u", ComboDisplayState.Combo);

					vec2 valuTextSize = fontRenderer.MeasureString(debugFont, valuText);
					vec2 comboTextPos = { valuTextPos.x + valuTextSize.x + 4.0f, valuTextPos.y };

					fontRenderer.PushString(debugFont, valuText, valuTextPos, vec2(1.0f), valuColor);
					fontRenderer.PushString(debugFont, std::string_view(comboText, comboTextLength), comboTextPos, vec2(1.0f), comboColor);
				}
			}
		}

		void DrawScoreBonusDisplay(float deltaTime_ms)
		{
			if (ScoreBonusDisplay.Value > 0 && ScoreBonusDisplay.ElapsedDisplayTime <= 1.0f)
			{
				const Font& debugFont = *mainGameContext.DebugFont;
				FontRenderer& fontRenderer = mainGameContext.SpriteRenderer->Font();

				char text[8] = {};
				size_t textLength = snprintf(text, sizeof(text) - 1, "+%u", ScoreBonusDisplay.Value);

				vec2 textSize = fontRenderer.MeasureString(debugFont, std::string_view(text, textLength));

				float textPosY = MathExtensions::ConvertRange(0.0f, 1.0f, 0.0f, -20.0f, ScoreBonusDisplay.ElapsedDisplayTime);
				vec2 textPos = { ScoreBonusDisplay.Position.x - textSize.x / 2.0f, ScoreBonusDisplay.Position.y + textPosY - 45.0f };

				fontRenderer.PushString(debugFont, std::string_view(text, textLength), textPos, vec2(1.0f), DefaultColors::White);
			}
		}

		void SetComboDisplayState(HitEvaluation hitEvaluation, u32 combo, vec2& position)
		{
			ComboDisplayState.Position = position;
			ComboDisplayState.HitEvaluation = hitEvaluation;
			ComboDisplayState.Combo = combo;
			ComboDisplayState.ElapsedDisplayTime = 0.0f;
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
		implementation = new Implementation(mainGameContext);
		implementation->Initialize();
	}

	bool HUD::LoadSprites(Starshine::GFX::SpritePacker& sprPacker)
	{
		return implementation->LoadSprites(sprPacker);
	}

	void HUD::Destroy()
	{
		delete implementation;
	}

	void HUD::Update(float deltaTime_ms)
	{
		implementation->UpdateScoreDisplay(deltaTime_ms);
		implementation->UpdateComboDisplay(deltaTime_ms);
		implementation->UpdateScoreBonusDisplay(deltaTime_ms);
	}

	void HUD::Draw(float deltaTime_ms)
	{
		implementation->DrawComboDisplay(deltaTime_ms);
		implementation->DrawScoreBonusDisplay(deltaTime_ms);
		implementation->DrawScoreDisplay();
	}

	void HUD::SetComboDisplayState(HitEvaluation hitEvaluation, u32 combo, vec2& position)
	{
		if (hitEvaluation != HitEvaluation::None)
		{
			implementation->SetComboDisplayState(hitEvaluation, combo, position);
		}
	}

	void HUD::SetScoreBonusDisplayState(u32 value, vec2& position)
	{
		if (value > 0)
		{
			implementation->SetScoreBonusDisplayState(value, position);
		}
	}

	void HUD::HoldScoreBonus()
	{
		implementation->HoldScoreBonusDisplay();
	}

	void HUD::ReleaseScoreBonus(bool drop)
	{
		implementation->ReleaseScoreBonusDisplay(drop);
	}
}
