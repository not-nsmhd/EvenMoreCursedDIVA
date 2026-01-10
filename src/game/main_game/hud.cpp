#include "hud.h"
#include "common/color.h"
#include <Common/MathExt.h>
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
			vec2 Position{ 1152.0f, 160.0f };
			u32 DisplayValue{};
			float IncrementSpeed{ 0.02f };
		} ScoreDisplay;

		Implementation(MainGame::Context& context) : mainGameContext{ context }
		{
		}

		void Initialize()
		{
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
			const Font& debugFont = *mainGameContext.DebugFont;
			FontRenderer& fontRenderer = mainGameContext.SpriteRenderer->Font();

			char scoreText[8] = {};
			size_t scoreTextLength = snprintf(scoreText, sizeof(scoreText), "%07u", ScoreDisplay.DisplayValue);

			fontRenderer.PushString(debugFont, std::string_view(scoreText, scoreTextLength), ScoreDisplay.Position, vec2(1.0f), DefaultColors::White);
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
