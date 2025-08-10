#include "hud.h"
#include "../common/color.h"
#include "../common/math_ext.h"
#include "../global_res.h"
#include <SDL2/SDL.h>

namespace MainGame
{
	using namespace DIVA;
	using namespace Common;
	using std::string_view;

	constexpr EnumStringMappingTable<HitEvaluation> HitEvaluationNames
	{
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
		GFX::Font* debugFont = nullptr;

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

		Implementation(MainGame::Context& context) : mainGameContext{ context }
		{
		}

		void Initialize()
		{
			debugFont = GlobalResources::DebugFont;
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

		void DrawComboDisplay(float deltaTime_ms)
		{
			if (ComboDisplayState.HitEvaluation != HitEvaluation::None && ComboDisplayState.ElapsedDisplayTime <= 2.0f)
			{
				string_view valuText = EnumToString<HitEvaluation>(HitEvaluationNames, ComboDisplayState.HitEvaluation);
				Color valuColor = HitEvaluationColors[static_cast<size_t>(ComboDisplayState.HitEvaluation)];
				Color comboColor = DefaultColors::White;

				vec2 valuTextPos = { ComboDisplayState.Position.x - 30.0f, ComboDisplayState.Position.y - 35.0f };

				if (ComboDisplayState.Combo <= 1)
				{
					debugFont->PushString(mainGameContext.SpriteRenderer, valuText, valuTextPos, vec2(1.0f), valuColor);
				}
				else
				{
					char comboText[8] = {};
					size_t comboTextLength = SDL_snprintf(comboText, sizeof(comboText) - 1, "%u", ComboDisplayState.Combo);

					vec2 valuTextSize = debugFont->MeasureString(valuText);
					vec2 comboTextPos = { valuTextPos.x + valuTextSize.x + 4.0f, valuTextPos.y };

					debugFont->PushString(mainGameContext.SpriteRenderer, valuText, valuTextPos, vec2(1.0f), valuColor);
					debugFont->PushString(mainGameContext.SpriteRenderer, comboText, comboTextLength, comboTextPos, vec2(1.0f), comboColor);
				}
			}
		}

		void DrawScoreBonusDisplay(float deltaTime_ms)
		{
			if (ScoreBonusDisplay.Value > 0 && ScoreBonusDisplay.ElapsedDisplayTime <= 1.0f)
			{
				char text[8] = {};
				size_t textLength = SDL_snprintf(text, sizeof(text) - 1, "+%u", ScoreBonusDisplay.Value);

				vec2 textSize = debugFont->MeasureString(text, textLength);

				float textPosY = MathExtensions::ConvertRange(0.0f, 1.0f, 0.0f, -20.0f, ScoreBonusDisplay.ElapsedDisplayTime);
				vec2 textPos = { ScoreBonusDisplay.Position.x - textSize.x / 2.0f, ScoreBonusDisplay.Position.y + textPosY - 45.0f };

				debugFont->PushString(mainGameContext.SpriteRenderer, text, textLength, textPos, vec2(1.0f), DefaultColors::White);
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
		implementation->UpdateComboDisplay(deltaTime_ms);
		implementation->UpdateScoreBonusDisplay(deltaTime_ms);
	}

	void HUD::Draw(float deltaTime_ms)
	{
		implementation->DrawComboDisplay(deltaTime_ms);
		implementation->DrawScoreBonusDisplay(deltaTime_ms);
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
