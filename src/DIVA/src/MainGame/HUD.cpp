#include "HUD.h"
#include <Graphics/SpritePacker.h>
#include <Common/Color.h>
#include <Common/MathExt.h>
#include <Graphics/AnimationSet.h>
#include <Rendering/Render2D/SpriteRenderer.h>
#include "../GameContext.h"

namespace DIVA::MainGame
{
	using namespace Starshine;
	using namespace Starshine::Graphics;
	using namespace Starshine::Rendering::Render2D;
	using std::string_view;

	struct HUD::Impl
	{
		MainGame::MainGameContext* mainGameContext = nullptr;
		Font* debugFont = nullptr;

		struct SpriteCache
		{
			std::shared_ptr<SpriteSheet> hudSprites;

			const Sprite* HitEvaluations[EnumCount<HitEvaluation>()]{};
			const Sprite* HitEvaluations_Wrong[EnumCount<HitEvaluation>()]{};

			const Sprite* ScoreNumbers[10]{};
			const Sprite* ComboNumbers[10]{};

			const Sprite* ScoreBonusNumbers[10]{};
			const Sprite* ScoreBonus_Plus{};
		} spriteCache;

		struct AnimationCache
		{
			std::unique_ptr<AnimationSet> hudAnimSet;

			const Animation* HitValu_Normal{};
			const Animation* HitValu_Miss{};

			const Animation* ScoreBonus{};

			Animation* FrameTop{};
			Animation* FrameBottom{};
			Layer* FrameTop_Difficulty{};

			vec2 SongNameTextPosition{};
			vec2 ScoreTextPosition{};
			vec2 LyricsTextPosition{};
		} animCache;

		struct ComboDisplayData
		{
			vec2 Position{};
			HitEvaluation HitEvaluation{};
			bool IsWrong{};
			u32 Combo{};

			f32 ElapsedDisplayTime{};
		} ComboDisplayState;

		struct ScoreBonusDisplayData
		{
			vec2 Position{};
			u32 Value{};
			bool Held{};

			f32 ElapsedDisplayTime{};
		} ScoreBonusDisplay;

		struct ScoreDisplayData
		{
			u32 DisplayValue{};
			f32 IncrementSpeed{ 0.02f };
		} ScoreDisplay;

		std::array<char, 256> LyricsTextBuffer{};
		Color LyricsColor{};
		vec2 LyricsTextDisplaySize{};

		Impl(MainGame::MainGameContext& context) : mainGameContext{ &context }
		{
		}

		~Impl()
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

			::memset(LyricsTextBuffer.data(), 0, LyricsTextBuffer.max_size());
			LyricsTextDisplaySize = {};
		}

		bool LoadSprites(Graphics::SpritePacker& sprPacker)
		{
			sprPacker.Clear();
			sprPacker.Initialize();

			sprPacker.AddFromDirectory("diva/sprites/mg_hud");
			sprPacker.Pack();

			spriteCache.hudSprites = std::make_shared<SpriteSheet>();
			spriteCache.hudSprites->CreateFromSpritePacker(sprPacker);
			sprPacker.Clear();

			auto fetchHitValueSprite = [&](HitEvaluation valu, std::string_view name, const Sprite* spriteArray[])
			{
				spriteArray[static_cast<size_t>(valu)] = &spriteCache.hudSprites->GetSprite(name);
			};

			auto fetchSprite = [&](std::string_view name)
			{
				return &spriteCache.hudSprites->GetSprite(name);
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
		
		bool LoadAnimations()
		{
			static constexpr std::string_view DifficultyLayerNames[EnumCount<Formats::ChartDifficulty>()]
			{
				"Difficulty_Easy",
				"Difficulty_Normal",
				"Difficulty_Hard",
				"Difficulty_Extreme"
			};

			animCache.hudAnimSet = std::make_unique<AnimationSet>();
			animCache.hudAnimSet->LoadXml("diva/sprites/mg_hud.xml");
			animCache.hudAnimSet->LinkToSpriteSheet(spriteCache.hudSprites);

			animCache.HitValu_Normal = &animCache.hudAnimSet->GetAnimation("HitValu_Normal");
			animCache.HitValu_Miss = &animCache.hudAnimSet->GetAnimation("HitValu_Miss");
			animCache.ScoreBonus = &animCache.hudAnimSet->GetAnimation("ScoreBonus");

			animCache.FrameTop = &animCache.hudAnimSet->GetAnimation("Frame_Top");
			animCache.FrameBottom = &animCache.hudAnimSet->GetAnimation("Frame_Bottom");

			animCache.FrameTop_Difficulty = &animCache.FrameTop->GetLayer("Difficulty");

			size_t difficultyIndex = static_cast<size_t>(mainGameContext->Difficulty);
			animCache.FrameTop_Difficulty->SpriteDefinition = &animCache.hudAnimSet->GetSpriteDefinition(DifficultyLayerNames[difficultyIndex]);

			Layer& songNameRef = animCache.FrameTop->GetLayer("SongName_Ref");
			songNameRef.Visible = false;
			animCache.SongNameTextPosition = songNameRef.GetTransform(0.0f).Position;

			Layer& scoreRef = animCache.FrameTop->GetLayer("Score_Ref");
			scoreRef.Visible = false;
			animCache.ScoreTextPosition = scoreRef.GetTransform(0.0f).Position;

			Layer& lyricsTextRef = animCache.FrameBottom->GetLayer("LyricsText_Ref");
			lyricsTextRef.Visible = false;
			animCache.LyricsTextPosition = lyricsTextRef.GetTransform(0.0f).Position;

			return true;
		}

		void UpdateScoreDisplay(const f32& frameTimeScale)
		{
			i32 scoreDiff = mainGameContext->Score.Score - ScoreDisplay.DisplayValue;

			if (scoreDiff < 61)
			{
				if (scoreDiff < 21)
					ScoreDisplay.DisplayValue = mainGameContext->Score.Score;
				else
					ScoreDisplay.DisplayValue += 7;
			}
			else
			{
				f32 incrementSpeed = frameTimeScale * ScoreDisplay.IncrementSpeed;
				ScoreDisplay.DisplayValue += static_cast<u32>((incrementSpeed * static_cast<f32>((scoreDiff * 10))));
			}
		}

		void UpdateComboDisplay(const f32& deltaTime)
		{
			const Animation* valuAnim = animCache.HitValu_Normal;

			if (ComboDisplayState.ElapsedDisplayTime <= valuAnim->EndTime)
			{
				ComboDisplayState.ElapsedDisplayTime += animCache.hudAnimSet->GetRelativeFrameTimeStep(deltaTime);
			}
		}

		void UpdateScoreBonusDisplay(const f32& deltaTime)
		{
			const Animation* scoreBonusAnim = animCache.ScoreBonus;

			if (ScoreBonusDisplay.ElapsedDisplayTime <= scoreBonusAnim->EndTime && !ScoreBonusDisplay.Held)
			{
				ScoreBonusDisplay.ElapsedDisplayTime += animCache.hudAnimSet->GetRelativeFrameTimeStep(deltaTime);
			}
		}
		
		// NOTE: Value text is displayed from right to left
		float DrawSpriteNumericValue(u32 value, const Sprite* spriteArray[], const vec2& position, const vec2& scale, float spacing, const Color& color, int length = -1)
		{
			SpriteSheetRenderer& sprRenderer = mainGameContext->SpriteRenderer->SpriteSheet();
			vec2 displayOffset{ 0.0f, 0.0f };

			int realLength = (length == -1) ? 10 : length;

			u32 remainingNumbers = value;
			for (int i = 0; i < realLength; i++)
			{
				if (remainingNumbers == 0 && i > 0 && length == -1) { break; }

				int sprIndex = remainingNumbers % 10;
				const Sprite* numSprite = spriteArray[sprIndex];

				sprRenderer.PushSprite(*spriteCache.hudSprites, *numSprite, position + displayOffset, scale, color);

				displayOffset.x -= spacing * scale.x;
				remainingNumbers /= 10;
			}

			return displayOffset.x;
		}

		float MeasureSpriteNumericValue(u32 value, float spacing, int length = -1)
		{
			float displayOffset{ 0.0f };

			int realLength = (length == -1) ? 10 : length;

			u32 remainingNumbers = value;
			for (int i = 0; i < realLength; i++)
			{
				if (remainingNumbers == 0 && i > 0 && length == -1) { return displayOffset; }

				displayOffset += spacing;
				remainingNumbers /= 10;
			}

			return displayOffset;
		}

		void DrawScoreDisplay()
		{
			DrawSpriteNumericValue(ScoreDisplay.DisplayValue, spriteCache.ScoreNumbers, animCache.ScoreTextPosition, vec2(1.0f), 25.0f, DefaultColors::White, -1);
		}

		void DrawComboDisplay()
		{
			const Animation* valuAnim = ComboDisplayState.HitEvaluation == HitEvaluation::Miss ? 
				animCache.HitValu_Miss : animCache.HitValu_Normal;

			if (ComboDisplayState.HitEvaluation != HitEvaluation::None && ComboDisplayState.ElapsedDisplayTime <= valuAnim->EndTime)
			{
				const Transform2D animTransform = valuAnim->GetLayer(0).GetTransform(ComboDisplayState.ElapsedDisplayTime);

				SpriteSheetRenderer& sprRenderer = mainGameContext->SpriteRenderer->SpriteSheet();

				size_t valuIndex = static_cast<size_t>(ComboDisplayState.HitEvaluation);

				const Sprite* valuSprite = ComboDisplayState.IsWrong ? spriteCache.HitEvaluations_Wrong[valuIndex] : 
					spriteCache.HitEvaluations[valuIndex];

				vec2 valuTextPos
				{ 
					ComboDisplayState.Position.x + animTransform.Position.x,
					ComboDisplayState.Position.y - 35.0f + animTransform.Position.y
				};

				if (ComboDisplayState.Combo <= 1)
				{
					sprRenderer.PushSprite(*spriteCache.hudSprites, *valuSprite, valuTextPos, animTransform.Scale, animTransform.Color);
				}
				else
				{
					constexpr float valuComboSpacing = 17.0f;

					float comboTextWidth = MeasureSpriteNumericValue(ComboDisplayState.Combo, 17.0f);

					vec2 comboTextPos = { valuTextPos.x + (valuComboSpacing + (comboTextWidth / 2.0f)) * animTransform.Scale.x, valuTextPos.y };
					valuTextPos.x -= ((comboTextWidth / 2.0f) + valuComboSpacing) * animTransform.Scale.x;

					sprRenderer.PushSprite(*spriteCache.hudSprites, *valuSprite, valuTextPos, animTransform.Scale, animTransform.Color);
					DrawSpriteNumericValue(ComboDisplayState.Combo, spriteCache.ComboNumbers, comboTextPos, animTransform.Scale, 17.0f, animTransform.Color);
				}
			}
		}

		void DrawScoreBonusDisplay()
		{
			const Animation* scoreBonusAnim = animCache.ScoreBonus;

			if (ScoreBonusDisplay.Value > 0 && ScoreBonusDisplay.ElapsedDisplayTime <= scoreBonusAnim->EndTime)
			{
				const Transform2D animTransform = scoreBonusAnim->GetLayer(0).GetTransform(ScoreBonusDisplay.ElapsedDisplayTime);

				SpriteSheetRenderer& sprRenderer = mainGameContext->SpriteRenderer->SpriteSheet();

				float textWidth = MeasureSpriteNumericValue(ScoreBonusDisplay.Value * 10, 15.0f);
				float plusWidth = spriteCache.ScoreBonus_Plus->SourceRectangle.Width;

				vec2 textPos
				{ 
					ScoreBonusDisplay.Position.x + animTransform.Position.x,
					ScoreBonusDisplay.Position.y - 70.0f + animTransform.Position.y
				};
				DrawSpriteNumericValue(ScoreBonusDisplay.Value, spriteCache.ScoreBonusNumbers, textPos, animTransform.Scale, 15.0f, animTransform.Color);

				textPos.x -= textWidth - plusWidth - 5.0f;
				sprRenderer.PushSprite(*spriteCache.hudSprites, *spriteCache.ScoreBonus_Plus, textPos, animTransform.Scale, animTransform.Color);
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

		void SetLyricsText(std::string_view text, const Color& color)
		{
			auto font = GameContext::GetInstance()->TestCJKFont.get();

			::strncpy_s(LyricsTextBuffer.data(), LyricsTextBuffer.max_size(), text.data(), LyricsTextBuffer.size());
			LyricsTextDisplaySize = mainGameContext->SpriteRenderer->Font().MeasureString(font, LyricsTextBuffer.data());
			LyricsColor = color;
		}

		void DrawLyricsText()
		{
			if (LyricsTextBuffer.size() > 0)
			{
				FontRenderer& fontRenderer = mainGameContext->SpriteRenderer->Font();
				auto font = GameContext::GetInstance()->TestCJKFont.get();

				fontRenderer.PushString(font, LyricsTextBuffer.data(), animCache.LyricsTextPosition, vec2(1.0f), LyricsColor);
			}
		}

		void DrawFrame()
		{
			AnimationSetRenderer& animSetRenderer = mainGameContext->SpriteRenderer->AnimationSet();
			FontRenderer& fontRenderer = mainGameContext->SpriteRenderer->Font();
			auto font = GameContext::GetInstance()->TestCJKFont.get();

			animSetRenderer.PushAnimation(animCache.hudAnimSet.get(), animCache.FrameTop, 0.0f);
			animSetRenderer.PushAnimation(animCache.hudAnimSet.get(), animCache.FrameBottom, 0.0f);
			DrawScoreDisplay();

			fontRenderer.PushString(font, mainGameContext->SongName, animCache.SongNameTextPosition, vec2(1.0f), DefaultColors::White);
			DrawLyricsText();
		}
	};

	HUD::HUD(MainGameContext& context) : mainGameContext(context)
	{
		impl = new Impl(context);
	}

	HUD::~HUD()
	{
	}

	void HUD::Initialize()
	{
		impl->Initialize();
	}

	void HUD::Reset()
	{
		impl->Reset();
	}

	bool HUD::LoadSprites(Graphics::SpritePacker& sprPacker)
	{
		if (!impl->LoadSprites(sprPacker))
			return false;

		if (!impl->LoadAnimations())
			return false;

		return true;
	}

	void HUD::Destroy()
	{
		delete impl;
		impl = nullptr;
	}

	void HUD::Update(Starshine::GameTime& gameTime)
	{
		f32 deltaTime = gameTime.ElapsedFrameTime.GetSeconds();

		impl->UpdateScoreDisplay(deltaTime / gameTime.TargetFrameTime.GetSeconds());
		impl->UpdateComboDisplay(deltaTime);
		impl->UpdateScoreBonusDisplay(deltaTime);
	}

	void HUD::Draw(Starshine::GameTime& gameTime)
	{
		impl->DrawComboDisplay();
		impl->DrawScoreBonusDisplay();
		impl->DrawFrame();
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

	void HUD::SetLyricsText(std::string_view text, const Color& color)
	{
		impl->SetLyricsText(text, color);
	}
}
