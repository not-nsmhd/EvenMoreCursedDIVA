#include <Common/Types.h>
#include <Common/MathExt.h>
#include "MainGame.h"
#include "Chart.h"
#include "GameNote.h"
#include "HitEvaluation.h"
#include "HUD.h"
#include <Input/Keyboard.h>
#include <Input/Gamepad.h>
#include "GFX/SpritePacker.h"
#include "GameContext.h"
#include "IO/Path/Directory.h"
#include "IO/Path/File.h"
#include "IO/Xml.h"
#include "audio/AudioEngine.h"
#include "Menu/ChartSelect.h"
#include <deque>

namespace DIVA::MainGame
{
	using namespace Starshine;
	using namespace Starshine::GFX;
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
		i32 results_optionIndex = 0;
		bool resultsSaved = false;

		static constexpr std::array<std::string_view, 2> results_OptionNames
		{
			"Retry",
			"Return to Chart Select"
		};

		SpriteRenderer* spriteRenderer{};
		Font* debugFont;

		float TrailScrollOffset = 0.0f;
		bool Paused = false;

		Chart songChart;
		size_t chartNoteOffset = 0;

		f64 ElapsedTime_Seconds = 0.0f;
		std::deque<GameNote> ActiveNotes;

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

		HUD hud = HUD(MainGameContext);

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

		SpritePacker sprPacker;

		char debugText[512] = {};

		Impl(MainGame::MainGameContext& context) : MainGameContext{ context }
		{
		}

		~Impl()
		{
		}

		void Initialize()
		{
			GFXDevice = Rendering::GetDevice();
			hud.Initialize();
		}

		void Reset()
		{
			chartNoteOffset = 0;
			ElapsedTime_Seconds = 0.0;
			ActiveNotes.clear();

			MainGameContext.Score.Score = 0;
			MainGameContext.Score.Combo = 0;
			MainGameContext.Score.MaxCombo = 0;

			hud.Reset();
		}

		bool CreateIconSetSpriteSheet()
		{
			sprPacker.AddFromDirectory("diva/sprites/iconset_dev");
			sprPacker.Pack();

			MainGameContext.IconSetSprites.SpriteSheet.CreateFromSpritePacker(sprPacker);
			sprPacker.Clear();

			auto& spriteCache = MainGameContext.IconSetSprites;
			auto& iconSet = MainGameContext.IconSetSprites.SpriteSheet;

			spriteCache.NoteTargetHand = &iconSet.GetSprite("TargetHand_Normal");
			spriteCache.Trail_Normal = &iconSet.GetSprite("Trail_Normal");
			spriteCache.Trail_CT = &iconSet.GetSprite("Trail_CT");

			auto fetchNoteShapeSpecificSprite = [&](NoteShape shape, std::string_view name, const Sprite* spriteArray[])
			{
				spriteArray[static_cast<size_t>(shape)] = &iconSet.GetSprite(name);
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

		bool LoadContent()
		{
			spriteRenderer = GameContext::GetInstance()->SpriteRenderer.get();
			MainGameContext.SpriteRenderer = spriteRenderer;

			debugFont = GameContext::GetInstance()->DebugFont.get();
			MainGameContext.DebugFont = debugFont;

			sprPacker.Initialize();
			CreateIconSetSpriteSheet();
			hud.LoadSprites(sprPacker);
			
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
			HitSound_Hold_LoopVoice.SetVolume(0.35f);
			
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

			MainGameContext.IconSetSprites.SpriteSheet.Destroy();

			ActiveNotes.clear();
			songChart.Clear();
		}

		void Destroy()
		{
			hud.Destroy();
		}

		GameNote* FindNoteToEvaluate()
		{
			for (auto& note : ActiveNotes)
			{
				if (!note.HasBeenHit && !note.Expired && !note.Expiring && !note.ShouldBeRemoved) { return &note; }
			}

			return nullptr;
		}

		void UpdateChart(f64 deltaTime_ms)
		{
			for (auto chartNote = songChart.Notes.cbegin() + chartNoteOffset; chartNote != songChart.Notes.cend(); chartNote++)
			{
				if (chartNote->AppearTime <= ElapsedTime_Seconds)
				{
					f32 flyTime = songChart.GetNoteTime(chartNote->AppearTime);
					const ChanceTime* nextCT = songChart.GetNextChanceTime(chartNote->AppearTime + flyTime);

					if (chartNote->Type == NoteType::HoldEnd) { chartNoteOffset++; break; }

					GameNote& newNote = ActiveNotes.emplace_back(*chartNote, MainGameContext);
					newNote.FlyTime = flyTime;

					if (nextCT != nullptr && chartNote->AppearTime + flyTime >= nextCT->StartTime && chartNote->AppearTime + flyTime <= nextCT->EndTime)
					{
						newNote.ActiveDuringChanceTime = true;
					}

					newNote.Trail.ScrollResetThreshold = MainGameContext.IconSetSprites.Trail_Normal->SourceRectangle.Width;

					if (chartNote->Type == NoteType::HoldStart && chartNote->NextNote != nullptr)
					{
						GameNote& holdEndNote = ActiveNotes.emplace_back(*chartNote->NextNote, MainGameContext);
						holdEndNote.FlyTime = songChart.GetNoteTime(chartNote->NextNote->AppearTime);
						holdEndNote.ElapsedTime = ElapsedTime_Seconds - chartNote->NextNote->AppearTime;

						holdEndNote.Trail.ScrollResetThreshold = newNote.Trail.ScrollResetThreshold;

						newNote.NextNote = &holdEndNote;
					}

					chartNoteOffset++;
					break;
				}
			}
		}

		void UpdateActiveNotes(f64 deltaTime_ms)
		{
			size_t noteIndex = 0;
			while (noteIndex < ActiveNotes.size())
			{
				GameNote* note = &ActiveNotes[noteIndex];

				if (note->Expiring && !note->Expired && !note->HasBeenHit)
				{
					note->Expired = true;
					MainGameContext.Score.Combo = 0;
					hud.SetComboDisplayState(HitEvaluation::Miss, 0, false, note->TargetPosition);
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
						hud.SetScoreBonusDisplayState(note->Hold.CurrentBonus, note->TargetPosition);
					}
					else
					{
						MainGameContext.Score.Score += note->Hold.CurrentBonus;
						note->Hold.CurrentBonus = 0;
					}
				}

				note->Update(deltaTime_ms);
				noteIndex++;
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
				{
					AudioEngine::GetInstance()->PlaySound(shape == NoteShape::Star ? HitSound_Star_Normal : HitSound_Normal, 0.25f);
				}
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
				{
					AudioEngine::GetInstance()->PlaySound(shape == NoteShape::Star ? HitSound_Star_Normal : HitSound_Normal, 0.25f);
				}
				return;
			}

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
				!note->HitWrong)
			{
				if ((note->HitEvaluation == HitEvaluation::Cool) || (note->HitEvaluation == HitEvaluation::Good) && note->DoubleTap.GiveBonus)
				{
					MainGameContext.Score.Score += 200;
					hud.SetScoreBonusDisplayState(200, note->TargetPosition);
				}
				AudioEngine::GetInstance()->PlaySound(shape == NoteShape::Star ? HitSound_Star_Double : HitSound_Double, 0.25f);
			}
			else if (note->Type == NoteType::HoldStart)
			{
				hud.HoldScoreBonus();
				hud.SetScoreBonusDisplayState(note->Hold.CurrentBonus, note->TargetPosition);

				HitSound_Hold_LoopVoice.SetSource(shape == NoteShape::Star ? HitSound_StarHold_Loop : HitSound_Hold_Loop);
				HitSound_Hold_LoopVoice.SetFramePosition(0);
				HitSound_Hold_LoopVoice.SetLoopState(true);
				HitSound_Hold_LoopVoice.SetPlaying(true);
			}
			else if (note->Type == NoteType::HoldEnd)
			{
				bool drop = (note->HitEvaluation != HitEvaluation::Cool) && (note->HitEvaluation != HitEvaluation::Good) || note->HitWrong;
				hud.ReleaseScoreBonus(drop);

				HitSound_Hold_LoopVoice.SetSource(shape == NoteShape::Star ? HitSound_StarHold_LoopEnd : HitSound_Hold_LoopEnd);
				HitSound_Hold_LoopVoice.SetFramePosition(0);
				HitSound_Hold_LoopVoice.SetLoopState(false);
				HitSound_Hold_LoopVoice.SetPlaying(true);
			}
			else
			{
				AudioEngine::GetInstance()->PlaySound(shape == NoteShape::Star ? HitSound_Star_Normal : HitSound_Normal, 0.25f);
			}

			MainGameContext.Score.MaxCombo = MathExtensions::Max(MainGameContext.Score.Combo, MainGameContext.Score.MaxCombo);
			hud.SetComboDisplayState(note->HitEvaluation, MainGameContext.Score.Combo, note->HitWrong, note->TargetPosition);
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
				hud.SetScoreBonusDisplayState(200, note->TargetPosition);
			}
			else if (note->Type == NoteType::HoldStart)
			{
				hud.HoldScoreBonus();
				hud.SetScoreBonusDisplayState(note->Hold.CurrentBonus, note->TargetPosition);
			}
			else if (note->Type == NoteType::HoldEnd)
			{
				bool drop = (note->HitEvaluation != HitEvaluation::Cool) && (note->HitEvaluation != HitEvaluation::Good) || note->HitWrong;
				hud.ReleaseScoreBonus(drop);
			}

			MainGameContext.Score.MaxCombo = MathExtensions::Max(MainGameContext.Score.Combo, MainGameContext.Score.MaxCombo);
			hud.SetComboDisplayState(note->HitEvaluation, MainGameContext.Score.Combo, note->HitWrong, note->TargetPosition);
		}

		void Update(f64 deltaTime_ms)
		{
			if (CurrentSubState == SubState::Results)
			{
				UpdateResults(deltaTime_ms);
				return;
			}

			if (ElapsedTime_Seconds >= songChart.Duration)
			{
				CurrentSubState = SubState::Results;
				return;
			}

			if (!Paused)
			{	
				if (MusicSource != SourceHandle::Invalid && MusicVoice.IsPlaying())
				{
					ElapsedTime_Seconds = static_cast<f64>(MusicVoice.GetFramePosition()) / 44100.0;
					ChartDeltaTime = deltaTime_ms / 1000.0;
				}
				else
				{
					ElapsedTime_Seconds += deltaTime_ms / 1000.0;
					ChartDeltaTime = deltaTime_ms / 1000.0;
				}

				UpdateChart(ChartDeltaTime);
				UpdateActiveNotes(deltaTime_ms);

				for (size_t i = 0; i < EnumCount<NoteShape>(); i++)
				{
					UpdateInputBinding(KeyboardBinds.Notes[i].EnumValue, KeyboardBinds.Notes[i].MappedValue);
				}
#if 0
				for (size_t i = 0; i < EnumCount<NoteShape>(); i++)
				{
					UpdateInputGamepadBinding(GamepadBinds.Notes[i].EnumValue, GamepadBinds.Notes[i].MappedValue);
				}
#endif
				hud.Update(deltaTime_ms);
			}

			if (Keyboard::IsAnyTapped(KeyboardBinds.Pause, nullptr, nullptr))
#if 0
			if (Gamepad::IsAnyButtonTapped(GamepadBinds.Pause, nullptr, nullptr))
#endif
			{
				Paused = !Paused;
				if (MusicSource != SourceHandle::Invalid)
				{
					MusicVoice.SetPlaying(!Paused);
				}
			}

			SDL_memset(debugText, 0, sizeof(debugText));

			size_t lastPos = 0;
			lastPos += SDL_snprintf(debugText + lastPos, sizeof(debugText) - 1, "Elapsed Time: %.03f\n", ElapsedTime_Seconds);
			lastPos += SDL_snprintf(debugText + lastPos, sizeof(debugText) - 1, "Chart Notes: %llu/%llu\n", chartNoteOffset, songChart.Notes.size());
			lastPos += SDL_snprintf(debugText + lastPos, sizeof(debugText) - 1, "Active Notes: %llu\n", ActiveNotes.size());
			if (Paused)
			{
				lastPos += SDL_snprintf(debugText + lastPos, sizeof(debugText) - 1, "PAUSED\n");
			}
		}

		void UpdateResults(f64 deltaTime_ms)
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
					GameInstance->SetState(std::make_unique<Menu::ChartSelect>());
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

		void Draw(f64 deltaTime_ms)
		{
			if (CurrentSubState == SubState::Results)
			{
				DrawResults();
				return;
			}

			GFXDevice->Clear(ClearFlags::ClearFlags_Color, Color{ 0, 24, 24, 255 }, 1.0f, 0);
			GFXDevice->SetFaceCullingState(false, PolygonOrientation::Clockwise);
			spriteRenderer->SetBlendMode(BlendMode::Normal);

			for (auto& note : ActiveNotes)
			{
				note.Draw(deltaTime_ms);
			}

			hud.Draw(deltaTime_ms);

			spriteRenderer->Font().PushString(debugFont, std::string_view(debugText), vec2(0.0f, 0.0f), vec2(1.0f), DefaultColors::White);

			spriteRenderer->RenderSprites(nullptr);
			GFXDevice->SwapBuffers();
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
			GFXDevice->SwapBuffers();
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
		impl->GameInstance = GameInstance;
		impl->Initialize();
		return true;
	}
	
	bool MainGameState::LoadContent()
	{
		impl->LoadChart(LoadSettings.ChartPath);
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
	
	void MainGameState::Update(f64 deltaTime_milliseconds)
	{
		impl->Update(deltaTime_milliseconds);
	}
	
	void MainGameState::Draw(f64 deltaTime_milliseconds)
	{
		impl->Draw(deltaTime_milliseconds);
	}

	std::string_view MainGameState::GetStateName() const
	{
		return "Main Game";
	}
}
