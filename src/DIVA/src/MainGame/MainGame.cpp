#include <Common/Types.h>
#include <Common/MathExt.h>
#include "../Definitions.h"
#include "MainGame.h"
#include "Chart.h"
#include "Lyrics.h"
#include "GameNote.h"
#include "HitEvaluation.h"
#include "HUD.h"
#include <Input/Keyboard.h>
#include <Input/Gamepad.h>
#include "Graphics/SpritePacker.h"
#include "GameContext.h"
#include "IO/Path/Directory.h"
#include "IO/Path/File.h"
#include "IO/Xml.h"
#include "audio/AudioEngine.h"
#include "Menu/ChartSelect.h"
#include "../Settings.h"
#include <deque>

namespace DIVA::MainGame
{
	using namespace Starshine;
	using namespace Starshine::Graphics;
	using namespace Starshine::Rendering;
	using namespace Starshine::Rendering::Render2D;
	using namespace Starshine::Audio;
	using namespace Starshine::Input;

	enum class SubState : i32
	{
		MainGame,
		Results
	};

	struct MainGameState::Impl
	{
		Starshine::GameInstance* GameInstance{};
		Device* GFXDevice = nullptr;

		MainGameContext& MainGameContext;

		SubState CurrentSubState{ SubState::MainGame };
		i32 pause_optionIndex = 0;
		i32 results_optionIndex = 0;
		bool resultsSaved = false;

		static constexpr std::array<std::string_view, 3> pauseMenu_OptionLabels
		{
			"Resume",
			"Retry",
			"Return to Chart Select"
		};

		static constexpr std::array<std::string_view, 2> results_OptionNames
		{
			"Retry",
			"Return to Chart Select"
		};

		SpriteRenderer* spriteRenderer{};
		Font* debugFont{};

		bool Paused = false;

		Chart songChart;
		size_t chartNoteOffset = 0;

		std::vector<Lyrics::Lyric> songLyrics;
		size_t songLyricsOffset = 0;

		TimeSpan ElapsedTime{};
		std::deque<GameNote> ActiveNotes;

		bool IsChanceTime{ false };
		const ChanceTime* NextChanceTime{ nullptr };
		size_t PassedChanceTimes{};

		struct KeyboardBindsData
		{
			KeyBind Pause = KeyBind{ SDLK_ESCAPE, Input::UnboundKey };
			EnumValueMappingTable<NoteShape, KeyBind> Notes
			{
				EnumValueMapping<NoteShape, KeyBind> { NoteShape::Circle, KeyBind{ SDLK_d, SDLK_l } },
				EnumValueMapping<NoteShape, KeyBind> { NoteShape::Cross, KeyBind{ SDLK_s, SDLK_k } },
				EnumValueMapping<NoteShape, KeyBind> { NoteShape::Square, KeyBind{ SDLK_a, SDLK_j } },
				EnumValueMapping<NoteShape, KeyBind> { NoteShape::Triangle, KeyBind{ SDLK_w, SDLK_i } },
				EnumValueMapping<NoteShape, KeyBind> { NoteShape::Star, KeyBind{ SDLK_f, SDLK_h } }
			};
		} KeyboardBinds;

		struct GamepadBindsData
		{
			GamepadBind Pause = GamepadBind{ GamepadButton::Options, GamepadButton::Unbound };
			EnumValueMappingTable<NoteShape, GamepadBind> Notes
			{
				EnumValueMapping<NoteShape, GamepadBind> { NoteShape::Circle, GamepadBind{ GamepadButton::Circle, GamepadButton::DPad_Right } },
				EnumValueMapping<NoteShape, GamepadBind> { NoteShape::Cross, GamepadBind{ GamepadButton::Cross, GamepadButton::DPad_Down } },
				EnumValueMapping<NoteShape, GamepadBind> { NoteShape::Square, GamepadBind{ GamepadButton::Square, GamepadButton::DPad_Left } },
				EnumValueMapping<NoteShape, GamepadBind> { NoteShape::Triangle, GamepadBind{ GamepadButton::Triangle, GamepadButton::DPad_Up } },

				// NOTE: Stars can be hit by pulling a stick on a gamepad or flicking on the screen (Vita)/touch panel (PS4)
				EnumValueMapping<NoteShape, GamepadBind> { NoteShape::Star, GamepadBind{ GamepadButton::Unbound, GamepadButton::Unbound } }
			};
		} GamepadBinds;

		std::unique_ptr<HUD> hud{};

		SourceHandle HitSound_Normal{};
		SourceHandle HitSound_Double{};

		SourceHandle HitSound_Star_Normal{};
		SourceHandle HitSound_Star_Double{};

		Voice HitSound_Hold_LoopVoice{};

		SourceHandle HitSound_Hold_Loop{};
		SourceHandle HitSound_Hold_LoopEnd{};

		SourceHandle HitSound_StarHold_Loop{};
		SourceHandle HitSound_StarHold_LoopEnd{};

		Voice MusicVoice{};
		SourceHandle MusicSource{};

		size_t CurrentMusicPosition{};
		size_t PreviousMusicPosition{};
		f64 ChartDeltaTime{};

		std::unique_ptr<SpritePacker> sprPacker;

		char debugText[512] = {};

		Impl(MainGame::MainGameContext& context) : MainGameContext{ context }
		{
		}

		~Impl()
		{
		}

		void Reset()
		{
			chartNoteOffset = 0;
			songLyricsOffset = 0;

			ActiveNotes.clear();

			MusicVoice.SetFramePosition(0);
			MusicVoice.SetVolume(0.5f);

			ElapsedTime = { 0 };

			MainGameContext.Score.Score = 0;
			MainGameContext.Score.Combo = 0;
			MainGameContext.Score.MaxCombo = 0;

			IsChanceTime = false;
			NextChanceTime = nullptr;
			PassedChanceTimes = 0;

			Paused = false;
			pause_optionIndex = 0;
			results_optionIndex = 0;

			CurrentSubState = SubState::MainGame;
			resultsSaved = false;

			hud->Reset();
		}

		void Initialize()
		{
			GFXDevice = Rendering::GetDevice();

			hud = std::make_unique<HUD>(MainGameContext);
			hud->Initialize();

			Reset();
			SetKeybinds();
		}

		void SetKeybinds()
		{
			KeyboardBinds.Notes[0].MappedValue = SettingsData.Input.MainGame_Circle;
			KeyboardBinds.Notes[1].MappedValue = SettingsData.Input.MainGame_Cross;
			KeyboardBinds.Notes[2].MappedValue = SettingsData.Input.MainGame_Square;
			KeyboardBinds.Notes[3].MappedValue = SettingsData.Input.MainGame_Triangle;
			KeyboardBinds.Notes[4].MappedValue = SettingsData.Input.MainGame_Star;
		}

		bool CreateIconSetSpriteSheet()
		{
			sprPacker = std::make_unique<SpritePacker>();
			sprPacker->Initialize();
			sprPacker->AddFromDirectory("diva/sprites/iconset_dev");
			sprPacker->Pack();

			MainGameContext.IconSetSprites.SpriteSheet = std::make_shared<SpriteSheet>();
			MainGameContext.IconSetSprites.SpriteSheet->CreateFromSpritePacker(*sprPacker);
			sprPacker->Clear();

			auto& spriteCache = MainGameContext.IconSetSprites;
			auto& iconSet = MainGameContext.IconSetSprites.SpriteSheet;

			spriteCache.NoteTargetHand = &iconSet->GetSprite("TargetHand_Normal");
			spriteCache.Trail_Normal = &iconSet->GetSprite("Trail_Normal");
			spriteCache.Trail_CT = &iconSet->GetSprite("Trail_CT");

			auto fetchNoteShapeSpecificSprite = [&](NoteShape shape, std::string_view name, const Sprite* spriteArray[])
			{
				spriteArray[static_cast<size_t>(shape)] = &iconSet->GetSprite(name);
			};

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "Target_Circle", spriteCache.NoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "Target_Cross", spriteCache.NoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "Target_Square", spriteCache.NoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "Target_Triangle", spriteCache.NoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Star, "Target_Star", spriteCache.NoteTargets);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "Icon_Circle", spriteCache.NoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "Icon_Cross", spriteCache.NoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "Icon_Square", spriteCache.NoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "Icon_Triangle", spriteCache.NoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Star, "Icon_Star", spriteCache.NoteIcons);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "Target_Circle_Double", spriteCache.DoubleNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "Target_Cross_Double", spriteCache.DoubleNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "Target_Square_Double", spriteCache.DoubleNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "Target_Triangle_Double", spriteCache.DoubleNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Star, "Target_Star_Double", spriteCache.DoubleNoteTargets);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "Icon_Circle_Double", spriteCache.DoubleNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "Icon_Cross_Double", spriteCache.DoubleNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "Icon_Square_Double", spriteCache.DoubleNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "Icon_Triangle_Double", spriteCache.DoubleNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Star, "Icon_Star_Double", spriteCache.DoubleNoteIcons);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "TargetHand_Circle", spriteCache.DoubleNoteTargetHands);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "TargetHand_Cross", spriteCache.DoubleNoteTargetHands);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "TargetHand_Square", spriteCache.DoubleNoteTargetHands);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "TargetHand_Triangle", spriteCache.DoubleNoteTargetHands);
			fetchNoteShapeSpecificSprite(NoteShape::Star, "TargetHand_Star", spriteCache.DoubleNoteTargetHands);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "Target_Circle_Hold", spriteCache.HoldNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "Target_Cross_Hold", spriteCache.HoldNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "Target_Square_Hold", spriteCache.HoldNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "Target_Triangle_Hold", spriteCache.HoldNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Star, "Target_Star_Hold", spriteCache.HoldNoteTargets);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "Icon_Circle_Hold", spriteCache.HoldNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "Icon_Cross_Hold", spriteCache.HoldNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "Icon_Square_Hold", spriteCache.HoldNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "Icon_Triangle_Hold", spriteCache.HoldNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Star, "Icon_Star_Hold", spriteCache.HoldNoteIcons);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "HoldTrail_Circle", spriteCache.HoldNoteTrails);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "HoldTrail_Cross", spriteCache.HoldNoteTrails);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "HoldTrail_Square", spriteCache.HoldNoteTrails);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "HoldTrail_Triangle", spriteCache.HoldNoteTrails);
			fetchNoteShapeSpecificSprite(NoteShape::Star, "HoldTrail_Star", spriteCache.HoldNoteTrails);
			return true;
		}

		bool LoadAnimations()
		{
			MainGameContext.IconSetAnimations.Animations = std::make_unique<AnimationSet>();
			MainGameContext.IconSetAnimations.Animations->LoadXml("diva/sprites/iconset.xml");
			MainGameContext.IconSetAnimations.Animations->LinkToSpriteSheet(MainGameContext.IconSetSprites.SpriteSheet);

			auto getAnimationLayer = [&](std::string_view animName, std::string_view layerName)
			{
				auto& anim = MainGameContext.IconSetAnimations.Animations->GetAnimation(animName);
				return &anim.GetLayer(layerName);
			};

			MainGameContext.IconSetAnimations.NoteAppearLayer = getAnimationLayer("Note_Appear", "Target");
			MainGameContext.IconSetAnimations.NoteDisappearLayer = getAnimationLayer("Note_Disappear", "Target");
			MainGameContext.IconSetAnimations.NoteAppearEffect = &MainGameContext.IconSetAnimations.Animations->GetAnimation("Note_AppearEffect");

			return true;
		}

		bool LoadContent()
		{
			spriteRenderer = GameContext::GetInstance()->SpriteRenderer.get();
			MainGameContext.SpriteRenderer = spriteRenderer;

			debugFont = GameContext::GetInstance()->DebugFont.get();
			MainGameContext.DebugFont = debugFont;

			CreateIconSetSpriteSheet();
			LoadAnimations();

			hud->LoadSprites(*sprPacker);
			
			HitSound_Normal = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Normal_Normal01.ogg");
			HitSound_Double = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Normal_Double01.ogg");

			HitSound_Star_Normal = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Star_Normal01.ogg");
			HitSound_Star_Double = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Star_Double01.ogg");

			HitSound_Hold_Loop = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Normal_Hold01_Loop.ogg");
			HitSound_Hold_LoopEnd = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Normal_Hold01_LoopEnd.ogg");

			HitSound_StarHold_Loop = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Star_Hold01_Loop.ogg");
			HitSound_StarHold_LoopEnd = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Star_Hold01_LoopEnd.ogg");

			HitSound_Hold_LoopVoice = AudioEngine::GetInstance()->AllocateVoice(HitSound_Hold_Loop);
			HitSound_Hold_LoopVoice.SetLoopState(true);
			HitSound_Hold_LoopVoice.SetVolume(0.135f);
			
			if (MusicSource != SourceHandle::Invalid)
			{
				MusicVoice.SetVolume(0.5f);
				MusicVoice.SetPlaying(true);
			}

			return true;
		}

		bool LoadChart(std::string_view chartPath)
		{
			return songChart.LoadXml(chartPath);
		}

		bool LoadLyrics(std::string_view lyricsPath)
		{
			return Lyrics::LoadXml(lyricsPath, songLyrics);
		}
		
		bool LoadMusic(std::string_view musicPath)
		{
			MusicSource = AudioEngine::GetInstance()->LoadStreamingSource(musicPath);
			if (MusicSource != SourceHandle::Invalid)
			{
				MusicVoice = AudioEngine::GetInstance()->AllocateVoice(MusicSource);
				return true;
			}
			return false;
		}

		void UnloadContent()
		{
			AudioEngine::GetInstance()->FreeVoice(MusicVoice);
			AudioEngine::GetInstance()->FreeVoice(HitSound_Hold_LoopVoice);

			AudioEngine::GetInstance()->UnloadSource(MusicSource);

			AudioEngine::GetInstance()->UnloadSource(HitSound_Normal);
			AudioEngine::GetInstance()->UnloadSource(HitSound_Double);

			AudioEngine::GetInstance()->UnloadSource(HitSound_Star_Normal);
			AudioEngine::GetInstance()->UnloadSource(HitSound_Star_Double);

			AudioEngine::GetInstance()->UnloadSource(HitSound_Hold_Loop);
			AudioEngine::GetInstance()->UnloadSource(HitSound_Hold_LoopEnd);

			AudioEngine::GetInstance()->UnloadSource(HitSound_StarHold_Loop);
			AudioEngine::GetInstance()->UnloadSource(HitSound_StarHold_LoopEnd);

			MainGameContext.IconSetAnimations.Animations = nullptr;
			MainGameContext.IconSetSprites.SpriteSheet = nullptr;
			sprPacker->Clear();
			sprPacker = nullptr;

			hud->Destroy();
			hud = nullptr;
		}

		void Destroy()
		{
			ActiveNotes.clear();
			songChart.Clear();
			songLyrics.clear();
		}

		GameNote* FindNoteToEvaluate()
		{
			for (auto& note : ActiveNotes)
			{
				if (!note.HasBeenHit && !note.Expired && !note.Expiring && !note.ShouldBeRemoved) { return &note; }
			}

			return nullptr;
		}

		void UpdateChart()
		{
			for (auto chartNote = songChart.Notes.cbegin() + chartNoteOffset; chartNote != songChart.Notes.cend(); chartNote++)
			{
				if (chartNote->AppearTime <= ElapsedTime)
				{
					TimeSpan flyTime = songChart.GetNoteTime(chartNote->AppearTime);
					const ChanceTime* nextCT = songChart.GetNextChanceTime(chartNote->AppearTime);

					if (chartNote->Type == NoteType::HoldEnd) { chartNoteOffset++; break; }

					GameNote& newNote = ActiveNotes.emplace_back(*chartNote, MainGameContext);
					newNote.FlyTime = flyTime;

					if (nextCT != nullptr && chartNote->AppearTime >= nextCT->StartTime && chartNote->AppearTime <= nextCT->EndTime)
					{
						newNote.ActiveDuringChanceTime = true;
					}

					newNote.Trail.ScrollResetThreshold = MainGameContext.IconSetSprites.Trail_Normal->SourceRectangle.Width;

					if (chartNote->Type == NoteType::HoldStart && chartNote->NextNote != nullptr)
					{
						GameNote& holdEndNote = ActiveNotes.emplace_back(*chartNote->NextNote, MainGameContext);
						holdEndNote.FlyTime = songChart.GetNoteTime(chartNote->NextNote->AppearTime);
						holdEndNote.ElapsedTime = ElapsedTime - chartNote->NextNote->AppearTime;

						holdEndNote.Trail.ScrollResetThreshold = newNote.Trail.ScrollResetThreshold;

						newNote.NextNote = &holdEndNote;
					}

					chartNoteOffset++;
					break;
				}
			}

			if (NextChanceTime == nullptr && songChart.ChanceTimes.size() > PassedChanceTimes)
			{
				NextChanceTime = songChart.GetNextChanceTime(ElapsedTime);
			}

			if (NextChanceTime != nullptr)
			{
				if (NextChanceTime->StartTime <= ElapsedTime)
				{
					if (NextChanceTime->EndTime <= ElapsedTime)
					{
						IsChanceTime = false;
						NextChanceTime = nullptr;
						PassedChanceTimes++;
					}
					else { IsChanceTime = true; }
				}
			}
		}

		void UpdateActiveNotes(GameTime& gameTime)
		{
			size_t noteIndex = 0;
			while (noteIndex < ActiveNotes.size())
			{
				GameNote* note = &ActiveNotes[noteIndex];

				if (note->Expiring && !note->Expired && !note->HasBeenHit)
				{
					note->Expired = true;
					MainGameContext.Score.Combo = 0;
					hud->SetComboDisplayState(HitEvaluation::Miss, 0, false, note->TargetPosition);
				}

				if (note->ShouldBeRemoved)
				{
					ActiveNotes.erase(ActiveNotes.cbegin() + noteIndex);
					continue;
				}

				if (note->Type == NoteType::HoldStart)
				{
					if (!note->NextNote->HasBeenHit)
					{
						hud->SetScoreBonusDisplayState(note->Hold.CurrentBonus, note->TargetPosition);
					}
					else
					{
						MainGameContext.Score.Score += note->Hold.CurrentBonus;
						note->Hold.CurrentBonus = 0;
					}
				}

				note->Update(gameTime);
				//UpdateNoteAutoplay(note);

				noteIndex++;
			}
		}

		void UpdateNoteAutoplay(GameNote* note)
		{
			if (note->ElapsedTime >= note->FlyTime && !note->HasBeenEvaluated())
			{
				switch (note->Type)
				{
				case NoteType::Normal:
					AudioEngine::GetInstance()->PlaySound(note->Shape == NoteShape::Star ? HitSound_Star_Normal : HitSound_Normal, 0.125f);
					break;
				case NoteType::Double:
					note->DoubleTap.Primary = true;
					note->DoubleTap.Alternative = true;
					AudioEngine::GetInstance()->PlaySound(note->Shape == NoteShape::Star ? HitSound_Star_Double : HitSound_Double, 0.125f);
					break;
				case NoteType::HoldStart:
					note->Hold.PrimaryHeld = true;
					note->Hold.BonusBaseValue = IsChanceTime ? ScoreValues::Cool * 2 : 0;
					hud->HoldScoreBonus();
					hud->SetScoreBonusDisplayState(note->Hold.CurrentBonus, note->TargetPosition);

					HitSound_Hold_LoopVoice.SetSource(note->Shape == NoteShape::Star ? HitSound_StarHold_Loop : HitSound_Hold_Loop);
					HitSound_Hold_LoopVoice.SetFramePosition(0);
					HitSound_Hold_LoopVoice.SetLoopState(true);
					HitSound_Hold_LoopVoice.SetPlaying(true);
					break;
				case NoteType::HoldEnd:
					note->Hold.PrimaryHeld = false;
					hud->ReleaseScoreBonus(false);
					HitSound_Hold_LoopVoice.SetPlaying(false);
					AudioEngine::GetInstance()->PlaySound(note->Shape == NoteShape::Star ? HitSound_StarHold_LoopEnd : HitSound_Hold_LoopEnd, 0.135f);
					break;
				}

				note->Evaluate(note->Shape);

				MainGameContext.Score.Score += ScoreValues::Cool * (IsChanceTime ? 2 : 1);
				MainGameContext.Score.Combo++;

				hud->SetComboDisplayState(note->HitEvaluation, MainGameContext.Score.Combo, note->HitWrong, note->TargetPosition);
				if (IsChanceTime)
					hud->SetScoreBonusDisplayState(ScoreValues::Cool * 2, note->TargetPosition);
			}
		}

		void UpdateInputBinding(NoteShape shape, const KeyBind& binding)
		{
			bool primTapped = false;
			bool altTapped = false;

			bool primDown = false;
			bool altDown = false;

			bool tapped = Keyboard::IsAnyTapped(binding, &primTapped, &altTapped);
			Keyboard::IsAnyDown(binding, &primDown, &altDown);

			bool released = Keyboard::IsAnyReleased(binding, nullptr, nullptr);

			if (!tapped && !released) { return; }

			GameNote* note = FindNoteToEvaluate();
			if (note == nullptr)
			{
				if (tapped)
					AudioEngine::GetInstance()->PlaySound(shape == NoteShape::Star ? HitSound_Star_Normal : HitSound_Normal, 0.125f);
				return;
			}

			switch (note->Type)
			{
			case NoteType::Normal:
				if (!tapped) { return; }
				break;
			case NoteType::Double:
			{
				if (primTapped) { note->DoubleTap.Primary = true; }
				if (altTapped) { note->DoubleTap.Alternative = true; }

				note->Hold.PrimaryHeld = primDown;
				note->Hold.AlternativeHeld = altDown;
				break;
			}
			case NoteType::HoldStart:
			{
				if (!tapped && released) { return; }

				note->Hold.PrimaryHeld = primDown;
				note->Hold.AlternativeHeld = altDown;
				break;
			}
			case NoteType::HoldEnd:
			{
				if (tapped && !released) { return; }

				note->Hold.PrimaryHeld = primDown;
				note->Hold.AlternativeHeld = altDown;
				break;
			}
			}

			bool evaluated = note->Evaluate(shape);
			if (!evaluated)
			{
				if (tapped) 
					AudioEngine::GetInstance()->PlaySound(shape == NoteShape::Star ? HitSound_Star_Normal : HitSound_Normal, 0.125f);
				return;
			}

			u32 noteScore = 0;

			switch (note->HitEvaluation)
			{
			case HitEvaluation::Cool:
				noteScore = note->HitWrong ? ScoreValues::CoolWrong : ScoreValues::Cool;
				MainGameContext.Score.Combo = note->HitWrong ? 0 : (MainGameContext.Score.Combo + 1);
				break;
			case HitEvaluation::Good:
				noteScore = note->HitWrong ? ScoreValues::GoodWrong : ScoreValues::Good;
				MainGameContext.Score.Combo = note->HitWrong ? 0 : (MainGameContext.Score.Combo + 1);
				break;
			case HitEvaluation::Safe:
				noteScore = note->HitWrong ? ScoreValues::SafeWrong : ScoreValues::Safe;
				MainGameContext.Score.Combo = 0;
				break;
			case HitEvaluation::Bad:
				noteScore = note->HitWrong ? ScoreValues::BadWrong : ScoreValues::Bad;
				MainGameContext.Score.Combo = 0;
				break;
			case HitEvaluation::Miss:
				MainGameContext.Score.Combo = 0;
				break;
			}

			if (IsChanceTime && !note->HitWrong)
			{
				noteScore *= 2;
			}

			MainGameContext.Score.Score += noteScore;

			if (note->Type == NoteType::Double &&
				!note->HitWrong)
			{
				if ((note->HitEvaluation == HitEvaluation::Cool) || (note->HitEvaluation == HitEvaluation::Good) && note->DoubleTap.GiveBonus)
				{
					MainGameContext.Score.Score += 200;
					hud->SetScoreBonusDisplayState(200 + (IsChanceTime ? noteScore : 0), note->TargetPosition);
				}
				AudioEngine::GetInstance()->PlaySound(shape == NoteShape::Star ? HitSound_Star_Double : HitSound_Double, 0.125f);
			}
			else if (note->Type == NoteType::HoldStart)
			{
				if (IsChanceTime) { note->Hold.BonusBaseValue = noteScore; }

				hud->HoldScoreBonus();
				hud->SetScoreBonusDisplayState(note->Hold.CurrentBonus, note->TargetPosition);

				HitSound_Hold_LoopVoice.SetSource(shape == NoteShape::Star ? HitSound_StarHold_Loop : HitSound_Hold_Loop);
				HitSound_Hold_LoopVoice.SetFramePosition(0);
				HitSound_Hold_LoopVoice.SetLoopState(true);
				HitSound_Hold_LoopVoice.SetPlaying(true);
			}
			else if (note->Type == NoteType::HoldEnd)
			{
				bool drop = (note->HitEvaluation != HitEvaluation::Cool) && (note->HitEvaluation != HitEvaluation::Good) || note->HitWrong;
				hud->ReleaseScoreBonus(drop);

				HitSound_Hold_LoopVoice.SetPlaying(false);
				AudioEngine::GetInstance()->PlaySound(shape == NoteShape::Star ? HitSound_StarHold_LoopEnd : HitSound_Hold_LoopEnd, 0.135f);
			}
			else
			{
				AudioEngine::GetInstance()->PlaySound(shape == NoteShape::Star ? HitSound_Star_Normal : HitSound_Normal, 0.125f);
				if (IsChanceTime)
				{
					hud->SetScoreBonusDisplayState(noteScore, note->TargetPosition);
				}
			}

			MainGameContext.Score.MaxCombo = MathExtensions::Max(MainGameContext.Score.Combo, MainGameContext.Score.MaxCombo);
			hud->SetComboDisplayState(note->HitEvaluation, MainGameContext.Score.Combo, note->HitWrong, note->TargetPosition);
		}

		void UpdateInputGamepadBinding(NoteShape shape, const GamepadBind& binding)
		{
			bool primTapped = false;
			bool altTapped = false;

			bool primDown = false;
			bool altDown = false;

			bool tapped = false;
			bool released = false;

			if (shape != NoteShape::Star)
			{
				tapped = Gamepad::IsAnyButtonTapped(binding, &primTapped, &altTapped);
				Gamepad::IsAnyButtonDown(binding, &primDown, &altDown);

				released = Gamepad::IsAnyButtonReleased(binding, nullptr, nullptr);
			}
			else
			{
				primTapped = Gamepad::IsStickPulled(GamepadStick::Left);
				altTapped = Gamepad::IsStickPulled(GamepadStick::Right);

				primDown = Gamepad::IsStickHeld(GamepadStick::Left);
				altDown = Gamepad::IsStickHeld(GamepadStick::Right);

				tapped = primTapped || altTapped;
				released = Gamepad::IsStickReleased(GamepadStick::Left) || Gamepad::IsStickReleased(GamepadStick::Right);
			}

			if (!tapped && !released) { return; }

			GameNote* note = FindNoteToEvaluate();
			if (note == nullptr) { return; }

			switch (note->Type)
			{
			case NoteType::Normal:
				if (!tapped) { return; }
				break;
			case NoteType::Double:
			{
				if (primTapped) { note->DoubleTap.Primary = true; }
				if (altTapped) { note->DoubleTap.Alternative = true; }

				note->Hold.PrimaryHeld = primDown;
				note->Hold.AlternativeHeld = altDown;
				break;
			}
			case NoteType::HoldStart:
			{
				if (!tapped && released) { return; }

				note->Hold.PrimaryHeld = primDown;
				note->Hold.AlternativeHeld = altDown;
				break;
			}
			case NoteType::HoldEnd:
			{
				if (tapped && !released) { return; }

				note->Hold.PrimaryHeld = primDown;
				note->Hold.AlternativeHeld = altDown;
				break;
			}
			}

			bool evaluated = note->Evaluate(shape);
			if (!evaluated) { return; }

			switch (note->HitEvaluation)
			{
			case HitEvaluation::Cool:
				MainGameContext.Score.Score += note->HitWrong ? ScoreValues::CoolWrong : ScoreValues::Cool;
				MainGameContext.Score.Combo = note->HitWrong ? 0 : (MainGameContext.Score.Combo + 1);
				break;
			case HitEvaluation::Good:
				MainGameContext.Score.Score += note->HitWrong ? ScoreValues::GoodWrong : ScoreValues::Good;
				MainGameContext.Score.Combo = note->HitWrong ? 0 : (MainGameContext.Score.Combo + 1);
				break;
			case HitEvaluation::Safe:
				MainGameContext.Score.Score += note->HitWrong ? ScoreValues::SafeWrong : ScoreValues::Safe;
				MainGameContext.Score.Combo = 0;
				break;
			case HitEvaluation::Bad:
				MainGameContext.Score.Score += note->HitWrong ? ScoreValues::BadWrong : ScoreValues::Bad;
				MainGameContext.Score.Combo = 0;
				break;
			case HitEvaluation::Miss:
				MainGameContext.Score.Combo = 0;
				break;
			}

			if (note->Type == NoteType::Double &&
				note->DoubleTap.GiveBonus &&
				!note->HitWrong &&
				((note->HitEvaluation == HitEvaluation::Cool) || (note->HitEvaluation == HitEvaluation::Good)))
			{
				MainGameContext.Score.Score += 200;
				hud->SetScoreBonusDisplayState(200, note->TargetPosition);
			}
			else if (note->Type == NoteType::HoldStart)
			{
				hud->HoldScoreBonus();
				hud->SetScoreBonusDisplayState(note->Hold.CurrentBonus, note->TargetPosition);
			}
			else if (note->Type == NoteType::HoldEnd)
			{
				bool drop = (note->HitEvaluation != HitEvaluation::Cool) && (note->HitEvaluation != HitEvaluation::Good) || note->HitWrong;
				hud->ReleaseScoreBonus(drop);
			}

			MainGameContext.Score.MaxCombo = MathExtensions::Max(MainGameContext.Score.Combo, MainGameContext.Score.MaxCombo);
			hud->SetComboDisplayState(note->HitEvaluation, MainGameContext.Score.Combo, note->HitWrong, note->TargetPosition);
		}

		void UpdateLyrics()
		{
			for (auto lyric = songLyrics.cbegin() + songLyricsOffset; lyric != songLyrics.cend(); lyric++)
			{
				if (lyric->StartTime <= ElapsedTime.GetSeconds())
				{
					if (lyric->EndTime <= ElapsedTime.GetSeconds())
					{
						hud->SetLyricsText("", DefaultColors::Transparent);
						songLyricsOffset++;
						continue;
					}
					hud->SetLyricsText(lyric->Text, lyric->Color);
				}
			}
		}

		void Update(GameTime& gameTime)
		{
			if (CurrentSubState == SubState::Results)
			{
				UpdateResults();
				return;
			}

			if (ElapsedTime >= songChart.Duration)
			{
				CurrentSubState = SubState::Results;
				return;
			}

			if (!Paused)
			{	
				if (MusicSource != SourceHandle::Invalid && MusicVoice.IsPlaying())
					ElapsedTime = TimeSpanConversion::FromSeconds(static_cast<f64>(MusicVoice.GetFramePosition() / 44100.0));
				else
					ElapsedTime += gameTime.ElapsedFrameTime;

				UpdateChart();
				UpdateLyrics();
				UpdateActiveNotes(gameTime);

				for (size_t i = 0; i < EnumCount<NoteShape>(); i++)
				{
					UpdateInputBinding(KeyboardBinds.Notes[i].EnumValue, KeyboardBinds.Notes[i].MappedValue);
				}
				hud->Update(gameTime);
			}
			else
			{
				UpdatePauseMenu();
			}

			if (Keyboard::IsAnyTapped(KeyboardBinds.Pause, nullptr, nullptr))
			{
				Paused = !Paused;
				if (MusicSource != SourceHandle::Invalid)
					MusicVoice.SetPlaying(!Paused);
			}

			SDL_memset(debugText, 0, sizeof(debugText));

			size_t lastPos = 0;
			lastPos += SDL_snprintf(debugText + lastPos, sizeof(debugText) - 1, "Elapsed Time: %.03f\n", ElapsedTime.GetSeconds());
			lastPos += SDL_snprintf(debugText + lastPos, sizeof(debugText) - 1, "Chart Notes: %llu/%llu\n", chartNoteOffset, songChart.Notes.size());
			lastPos += SDL_snprintf(debugText + lastPos, sizeof(debugText) - 1, "Active Notes: %llu\n", ActiveNotes.size());
		}

		void UpdatePauseMenu()
		{
			if (Keyboard::IsKeyTapped(SDLK_DOWN)) { pause_optionIndex++; }
			if (Keyboard::IsKeyTapped(SDLK_UP)) { pause_optionIndex--; }
			pause_optionIndex = MathExtensions::Clamp<i32>(pause_optionIndex, 0, 2);

			if (Keyboard::IsKeyTapped(SDLK_RETURN))
			{
				switch (pause_optionIndex)
				{
				case 0:
					Paused = false;
					if (MusicSource != SourceHandle::Invalid) { MusicVoice.SetPlaying(!Paused); }
					break;
				case 1:
					Reset();
					Paused = false;
					if (MusicSource != SourceHandle::Invalid) { MusicVoice.SetPlaying(!Paused); }
					break;
				case 2:
					GameInstance->SetState(GetStatePointer(StateID::ChartSelect));
					return;
				}

				pause_optionIndex = 0;
				return;
			}
		}

		void UpdateResults()
		{
			if (!resultsSaved)
			{
				Xml::Printer printer;

				printer.OpenElement("Score");
				{
					printer.PushAttribute("Score", MainGameContext.Score.Score);
					printer.PushAttribute("MaxCombo", MainGameContext.Score.MaxCombo);

					printer.CloseElement();
				}

				IO::File::WriteAllBytes("userdata/score_test.xml", printer.CStr(), printer.CStrSize() - 1);
				printer.ClearBuffer();
				
				resultsSaved = true;
			}

			if (Keyboard::IsKeyTapped(SDLK_DOWN)) { results_optionIndex++; }
			if (Keyboard::IsKeyTapped(SDLK_UP)) { results_optionIndex--; }
			results_optionIndex = MathExtensions::Clamp<i32>(results_optionIndex, 0, 1);

			if (Keyboard::IsKeyTapped(SDLK_RETURN))
			{ 
				switch (results_optionIndex)
				{
				case 0:
					CurrentSubState = SubState::MainGame;
					resultsSaved = false;
					Reset();
					break;
				case 1:
					GameInstance->SetState(GetStatePointer(StateID::ChartSelect));
					return;
				}

				results_optionIndex = 0;
				return;
			}

			SDL_memset(debugText, 0, sizeof(debugText));

			size_t lastPos = 0;
			lastPos += SDL_snprintf(debugText + lastPos, sizeof(debugText) - 1, "Score: %d\n", MainGameContext.Score.Score);
			lastPos += SDL_snprintf(debugText + lastPos, sizeof(debugText) - 1, "Max Combo: %d\n", MainGameContext.Score.MaxCombo);
		}

		void Draw(GameTime& gameTime)
		{
			if (CurrentSubState == SubState::Results)
			{
				DrawResults();
				return;
			}

			const RectangleF viewportSize = GFXDevice->GetViewportSize();
			const vec2 baseScale = vec2(viewportSize.Width / 1280.0f, viewportSize.Height / 720.0f);

			GFXDevice->Clear(ClearFlags::ClearFlags_Color, Color{ 0, 24, 24, 255 }, 1.0f, 0);
			spriteRenderer->SetBlendMode(BlendMode::Normal);
			spriteRenderer->SetBasePositionAndScale({}, baseScale);

			for (auto& note : ActiveNotes)
			{
				note.UpdateTrail();
				note.DrawTrail();
			}

			for (auto& note : ActiveNotes)
			{
				note.Draw(gameTime);
			}

			hud->Draw(gameTime);

			spriteRenderer->SetBasePositionAndScale({}, baseScale);
			spriteRenderer->RenderSprites(nullptr);

			spriteRenderer->Font().PushString(debugFont, std::string_view(debugText), vec2(0.0f, 0.0f), vec2(1.0f), DefaultColors::White);

			if (Paused)
				DrawPauseMenu();

			spriteRenderer->RenderSprites(nullptr);
		}

		void DrawPauseMenu()
		{
			const RectangleF viewportSize = GFXDevice->GetViewportSize();
			const vec2 menuPosition = { viewportSize.Width / 2.0f, (viewportSize.Height / 2.0f) - debugFont->LineHeight * 3.0f };

			spriteRenderer->SetSpriteColor({ 0, 0, 0, 128 });
			spriteRenderer->SetSpritePosition(viewportSize.Position());
			spriteRenderer->SetSpriteSize(viewportSize.Size());
			spriteRenderer->PushSprite(nullptr);

			for (i32 i = 0; i < pauseMenu_OptionLabels.size(); i++)
			{
				spriteRenderer->Font().PushString(debugFont, pauseMenu_OptionLabels[i],
					vec2(menuPosition.x, menuPosition.y + static_cast<float>(i) * debugFont->LineHeight), vec2(1.0f),
					i == pause_optionIndex ? DefaultColors::Yellow : DefaultColors::White);
			}
		}

		void DrawResults()
		{
			GFXDevice->Clear(ClearFlags::ClearFlags_Color, Color{ 0, 24, 24, 255 }, 1.0f, 0);
			spriteRenderer->SetBlendMode(BlendMode::Normal);

			spriteRenderer->Font().PushString(debugFont, std::string_view(debugText), vec2(0.0f, 0.0f), vec2(1.0f), DefaultColors::White);

			for (i32 i = 0; i < results_OptionNames.size(); i++)
			{
				spriteRenderer->Font().PushString(debugFont, results_OptionNames[i], vec2(0.0f, 40.0f + static_cast<float>(i) * debugFont->LineHeight), vec2(1.0f),
					i == results_optionIndex ? DefaultColors::Yellow : DefaultColors::White);
			}

			spriteRenderer->RenderSprites(nullptr);
		}
	};

	MainGameState::MainGameState() : impl(std::make_unique<Impl>(context))
	{
	}

	MainGameState::~MainGameState()
	{
	}
	
	bool MainGameState::Initialize()
	{
		impl->MainGameContext.SongName = LoadSettings.SongName;
		impl->MainGameContext.Difficulty = LoadSettings.Difficulty;

		impl->GameInstance = GameInstance;
		impl->Initialize();
		return true;
	}
	
	bool MainGameState::LoadContent()
	{
		impl->LoadChart(LoadSettings.ChartPath);
		impl->LoadLyrics(LoadSettings.LyricsPath);
		impl->LoadMusic(LoadSettings.MusicPath);
		return impl->LoadContent();
	}
	
	void MainGameState::UnloadContent()
	{
		impl->UnloadContent();
	}
	
	void MainGameState::Destroy()
	{
		impl->Destroy();
	}
	
	void MainGameState::Update(GameTime& gameTime)
	{
		impl->Update(gameTime);
	}
	
	void MainGameState::Draw(GameTime& gameTime)
	{
		impl->Draw(gameTime);
	}
}
