#include "MainGame.h"
#include "Chart.h"
#include "HitEvaluation.h"
#include "HUD.h"
#include <Common/Types.h>
#include <Common/MathExt.h>
#include "input/Keyboard.h"
#include "gfx/Render2D/SpriteRenderer.h"
#include "GFX/SpritePacker.h"
#include "IO/Path/Directory.h"
#include "audio/AudioEngine.h"
#include <string>
#include <deque>
#include <fstream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

namespace DIVA::MainGame
{
	using namespace Starshine;
	using namespace Starshine::GFX;
	using namespace Starshine::GFX::Render2D;
	using namespace Starshine::Audio;
	using namespace Starshine::Input;
	using std::string_view;
	using std::fstream;
	using std::ios_base;

	constexpr vec2 DefaultIconPosition { std::numeric_limits<float>().infinity(), std::numeric_limits<float>().infinity() };
	constexpr float MaxNoteLifetime { 1000.0f };
	constexpr float TrailResolution { 1.0f / 48.0f };
	constexpr float TrailLengthFactor { 64.0f / 1.25f };
	constexpr float TrailScrollSpeed { 64.0f * TrailResolution };

	constexpr Color NoteTrailColors[EnumCount<NoteShape>()]
	{
		{ 237,  68,  78, 255 },
		{ 181, 255, 255, 255 },
		{ 255, 206, 255, 255 },
		{ 242, 255, 175, 255 }
	};

	struct GameNote
	{
		float NoteTime{};
		float ElapsedTime{};

		float RemainingTimeOnHit{};

		NoteShape Shape{};
		NoteType Type{};
		vec2 TargetPosition{};
		vec2 IconPosition{ DefaultIconPosition };

		float EntryAngle{};
		float Frequency{};
		float Amplitude{};
		float Distance{};

		// NOTE: Generic state flags
		bool Expired = false;
		bool HasBeenHit = false;
		bool ShouldBeRemoved = false;

		// NOTE: Double-specific state flags
		bool HasBeenHitPrimary = false;
		bool HasBeenHitAlternative = false;

		// NOTE: Used by hold notes (and possibly linked star notes if they get added)
		GameNote* NextNote = nullptr;
		u32 CurrentScoreBonus = 0;

		// NOTE: Hit Stats
		HitEvaluation HitEvaluation{ HitEvaluation::None };
		bool HitWrong = false;

		// NOTE: Functions
		// NOTE: Returned value is specified in seconds
		float GetRemainingTime() const { return NoteTime - ElapsedTime; }
		float GetNormalizedElapsedTime() const { return MathExtensions::ConvertRange(0.0f, NoteTime, 0.0f, 1.0f, ElapsedTime); }

		bool HasBeenEvaluated() const { return HitEvaluation != HitEvaluation::None; }
	};

	struct MainGameState::StateInternal
	{
		GFX::Renderer* BaseRenderer = nullptr;
		Context& MainGameContext;

		SpriteRenderer* spriteRenderer{};
		SpriteSheet iconSet;
		Font debugFont;

		float TrailScrollOffset = 0.0f;
		bool Paused = false;

		struct SpriteCache
		{
			const Sprite* NoteTargets[EnumCount<NoteShape>()]{};
			const Sprite* NoteIcons[EnumCount<NoteShape>()]{};
			const Sprite* NoteTargetHand{};

			const Sprite* DoubleNoteTargets[EnumCount<NoteShape>()]{};
			const Sprite* DoubleNoteIcons[EnumCount<NoteShape>()]{};
			const Sprite* DoubleNoteTargetHands[EnumCount<NoteShape>()]{};

			const Sprite* HoldNoteTargets[EnumCount<NoteShape>()]{};
			const Sprite* HoldNoteIcons[EnumCount<NoteShape>()]{};

			const Sprite* HoldNoteTrails[EnumCount<NoteShape>()]{};

			const Sprite* Trail_Normal{};
			const Sprite* Trail_CT{};
		} spriteCache;

		Chart songChart;
		size_t chartNoteOffset = 0;

		float ElapsedTime_Seconds = 0.0f;
		std::deque<GameNote> ActiveNotes;

		KeyBind AdvanceKeybind = KeyBind{ SDLK_PERIOD, Input::UnboundKey };
		KeyBind PauseKeybind = KeyBind{ SDLK_ESCAPE, Input::UnboundKey };
		EnumValueMappingTable<NoteShape, KeyBind> NoteKeybinds
		{
			EnumValueMapping<NoteShape, KeyBind> { NoteShape::Circle, KeyBind{ SDLK_d, SDLK_l } },
			EnumValueMapping<NoteShape, KeyBind> { NoteShape::Cross, KeyBind{ SDLK_s, SDLK_k } },
			EnumValueMapping<NoteShape, KeyBind> { NoteShape::Square, KeyBind{ SDLK_a, SDLK_j } },
			EnumValueMapping<NoteShape, KeyBind> { NoteShape::Triangle, KeyBind{ SDLK_w, SDLK_i } }
		};

		HUD hud = HUD(MainGameContext);

		SourceHandle HitSound_Normal{};
		SourceHandle HitSound_Double{};

		Voice HitSound_Hold_LoopVoice{};
		SourceHandle HitSound_Hold_Loop{};
		SourceHandle HitSound_Hold_LoopEnd{};

		Voice MusicVoice{};
		SourceHandle MusicSource{};

		size_t CurrentMusicPosition{};
		size_t PreviousMusicPosition{};
		float ChartDeltaTime{};

		char debugText[512] = {};

		StateInternal(MainGame::Context& context) : MainGameContext{ context }
		{
		}

		void Initialize()
		{
			BaseRenderer = Renderer::GetInstance();
			hud.Initialize();
		}

		bool CreateIconSetSpriteSheet()
		{
			SpritePacker sprPacker;
			sprPacker.Initialize();

			IO::Directory::IterateFiles("diva/sprites/iconset_dev",
				[&](std::string_view filePath)
				{
					sprPacker.AddImage(filePath);
				});

			sprPacker.Pack();

			iconSet.CreateFromSpritePacker(sprPacker);
			sprPacker.Clear();

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
			//fetchNoteShapeSpecificSprite(NoteShape::Star, "Target_Star", spriteCache.NoteTargets);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "Icon_Circle", spriteCache.NoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "Icon_Cross", spriteCache.NoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "Icon_Square", spriteCache.NoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "Icon_Triangle", spriteCache.NoteIcons);
			//fetchNoteShapeSpecificSprite(NoteShape::Star, "Icon_Star", spriteCache.NoteIcons);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "Target_Circle_Double", spriteCache.DoubleNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "Target_Cross_Double", spriteCache.DoubleNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "Target_Square_Double", spriteCache.DoubleNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "Target_Triangle_Double", spriteCache.DoubleNoteTargets);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "Icon_Circle_Double", spriteCache.DoubleNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "Icon_Cross_Double", spriteCache.DoubleNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "Icon_Square_Double", spriteCache.DoubleNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "Icon_Triangle_Double", spriteCache.DoubleNoteIcons);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "TargetHand_Circle", spriteCache.DoubleNoteTargetHands);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "TargetHand_Cross", spriteCache.DoubleNoteTargetHands);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "TargetHand_Square", spriteCache.DoubleNoteTargetHands);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "TargetHand_Triangle", spriteCache.DoubleNoteTargetHands);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "Target_Circle_Hold", spriteCache.HoldNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "Target_Cross_Hold", spriteCache.HoldNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "Target_Square_Hold", spriteCache.HoldNoteTargets);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "Target_Triangle_Hold", spriteCache.HoldNoteTargets);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "Icon_Circle_Hold", spriteCache.HoldNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "Icon_Cross_Hold", spriteCache.HoldNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "Icon_Square_Hold", spriteCache.HoldNoteIcons);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "Icon_Triangle_Hold", spriteCache.HoldNoteIcons);

			fetchNoteShapeSpecificSprite(NoteShape::Circle, "HoldTrail_Circle", spriteCache.HoldNoteTrails);
			fetchNoteShapeSpecificSprite(NoteShape::Cross, "HoldTrail_Cross", spriteCache.HoldNoteTrails);
			fetchNoteShapeSpecificSprite(NoteShape::Square, "HoldTrail_Square", spriteCache.HoldNoteTrails);
			fetchNoteShapeSpecificSprite(NoteShape::Triangle, "HoldTrail_Triangle", spriteCache.HoldNoteTrails);
			return true;
		}

		bool LoadContent()
		{
			spriteRenderer = new SpriteRenderer();
			MainGameContext.SpriteRenderer = spriteRenderer;

			debugFont.ReadBMFont("diva/fonts/debug.fnt");
			MainGameContext.DebugFont = &debugFont;

			CreateIconSetSpriteSheet();
			
			HitSound_Normal = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Normal_Normal01.ogg");
			HitSound_Double = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Normal_Double01.ogg");

			HitSound_Hold_Loop = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Normal_Hold01_Loop.ogg");
			HitSound_Hold_LoopEnd = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Normal_Hold01_LoopEnd.ogg");

			HitSound_Hold_LoopVoice = AudioEngine::GetInstance()->AllocateVoice(HitSound_Hold_Loop);
			HitSound_Hold_LoopVoice.SetLoopState(true);
			HitSound_Hold_LoopVoice.SetVolume(0.35f);

			MusicSource = AudioEngine::GetInstance()->LoadStreamingSource("diva/music/pv_022.ogg");
			MusicVoice = AudioEngine::GetInstance()->AllocateVoice(MusicSource);

			// ---------------

			fstream chartFile = fstream("diva/songdata/test/test_dt2_chart.xml", ios_base::binary | ios_base::in);
			chartFile.seekg(0, ios_base::end);
			size_t chartFileSize = chartFile.tellg();
			chartFile.seekg(0, ios_base::beg);

			char* chartFileText = new char[chartFileSize];
			chartFile.read(chartFileText, chartFileSize);
			chartFile.close();

			ReadXmlChart(songChart, chartFileText, chartFileSize);

			delete[] chartFileText;

			return true;
		}

		void UnloadContent()
		{
			ActiveNotes.clear();
			songChart.Clear();
			iconSet.Destroy();
		}

		void Destroy()
		{
			hud.Destroy();
		}

		void Update(float deltaTime_ms)
		{
			if (ElapsedTime_Seconds >= songChart.Duration)
			{
				return;
			}

			if (!Paused)
			{	
				MusicVoice.SetPlaying(true);

				if (MusicVoice.IsPlaying())
				{
					PreviousMusicPosition = CurrentMusicPosition;
					CurrentMusicPosition = MusicVoice.GetFramePosition();

					ChartDeltaTime = static_cast<float>(CurrentMusicPosition - PreviousMusicPosition) / 44100.0f;
					ElapsedTime_Seconds += ChartDeltaTime;
				}
				else
				{
					ElapsedTime_Seconds += deltaTime_ms / 1000.0f;
					ChartDeltaTime = deltaTime_ms / 1000.0f;
				}

				UpdateChart(ChartDeltaTime);

				size_t noteIndex = 0;
				while (noteIndex < ActiveNotes.size())
				{
					GameNote* note = &ActiveNotes[noteIndex];

					if (note->ShouldBeRemoved)
					{
						ActiveNotes.erase(ActiveNotes.cbegin() + noteIndex);
						continue;
					}

					UpdateNote(*note, ChartDeltaTime);
					noteIndex++;
				}

				for (size_t i = 0; i < EnumCount<NoteShape>(); i++)
				{
					UpdateInputBinding(NoteKeybinds[i].EnumValue, NoteKeybinds[i].MappedValue);
				}

				TrailScrollOffset += TrailScrollSpeed * (16.6667f / deltaTime_ms);
				TrailScrollOffset = SDL_fmodf(TrailScrollOffset, spriteCache.Trail_Normal->SourceRectangle.Width);

				hud.Update(deltaTime_ms);
			}

			if (Keyboard::IsAnyTapped(PauseKeybind, nullptr, nullptr))
			{
				Paused = !Paused;
				if (Paused)
				{
					MusicVoice.SetPlaying(false);
				}
				else
				{
					MusicVoice.SetPlaying(true);
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

		void Draw(float deltaTime_ms)
		{
			BaseRenderer->Clear(ClearFlags::ClearFlags_Color, Color{ 0, 24, 24, 255 }, 1.0f, 0);
			spriteRenderer->SetBlendMode(BlendMode::Normal);

			for (auto& note : ActiveNotes)
			{
				DrawNote(note);
			}

			hud.Draw(deltaTime_ms);

			spriteRenderer->Font().PushString(debugFont, std::string_view(debugText), vec2(0.0f, 0.0f), vec2(1.0f), DefaultColors::White);

			spriteRenderer->RenderSprites(nullptr);
			BaseRenderer->SwapBuffers();
		}

		void UpdateChart(float deltaTime_ms)
		{
			for (auto chartNote = songChart.Notes.cbegin() + chartNoteOffset; chartNote != songChart.Notes.cend(); chartNote++)
			{
				if (chartNote->AppearTime <= ElapsedTime_Seconds)
				{
					if (chartNote->Type == NoteType::HoldEnd)
					{
						chartNoteOffset++;
						break;
					}

					GameNote& newNote = ActiveNotes.emplace_back();
					newNote.NoteTime = songChart.GetNoteTime(chartNote->AppearTime);
					newNote.ElapsedTime = 0.0f;
					newNote.Shape = chartNote->Shape;
					newNote.Type = chartNote->Type;
					newNote.TargetPosition = { chartNote->X, chartNote->Y };
					newNote.IconPosition = DefaultIconPosition;
					newNote.EntryAngle = chartNote->Angle;
					newNote.Frequency = chartNote->Frequency;
					newNote.Amplitude = chartNote->Amplitude;
					newNote.Distance = chartNote->Distance;
					chartNoteOffset++;

					if (chartNote->Type == NoteType::HoldStart)
					{
						float originalAppearTime = chartNote->AppearTime;

						chartNote = songChart.Notes.cbegin() + chartNote->NextNoteIndex;

						GameNote& holdEndNote = ActiveNotes.emplace_back();
						holdEndNote.NoteTime = songChart.GetNoteTime(chartNote->AppearTime);
						holdEndNote.ElapsedTime = -(chartNote->AppearTime - originalAppearTime);
						holdEndNote.Shape = chartNote->Shape;
						holdEndNote.Type = NoteType::HoldEnd;
						holdEndNote.TargetPosition = { chartNote->X, chartNote->Y };
						holdEndNote.IconPosition = DefaultIconPosition;
						holdEndNote.EntryAngle = chartNote->Angle;
						holdEndNote.Frequency = chartNote->Frequency;
						holdEndNote.Amplitude = chartNote->Amplitude;
						holdEndNote.Distance = chartNote->Distance;
						newNote.NextNote = &holdEndNote;
					}

					break;
				}
			}
		}

		void UpdateNote(GameNote& note, float deltaTime_ms)
		{
			note.ElapsedTime += deltaTime_ms;

			bool isHoldNote = (NoteTypeToNoteTypeFlags(note.Type) & NoteTypeFlags_HoldAll) != 0;
			float remainingTime = note.GetRemainingTime() * 1000.0f;
			if (!isHoldNote)
			{
				if (!note.HasBeenEvaluated() && !note.Expired && !note.ShouldBeRemoved && note.ElapsedTime > 0.0f)
				{
					if (remainingTime < HitThresholds::ThresholdMiss)
					{
						note.Expired = true;
					}
				}

				if (remainingTime <= -MaxNoteLifetime)
				{
					note.ShouldBeRemoved = true;
				}
			}
			else if (note.Type == NoteType::HoldStart)
			{
				GameNote* nextNote = note.NextNote;
				float nextNoteRemainingTime = nextNote->GetRemainingTime() * 1000.0f;
				if (!note.Expired && !note.ShouldBeRemoved && !note.HasBeenEvaluated())
				{
					if (remainingTime < HitThresholds::ThresholdMiss)
					{
						note.Expired = true;
						nextNote->Expired = true;
						nextNote->ShouldBeRemoved = true;
					}
				}
				else if (note.HasBeenEvaluated())
				{
					if (!nextNote->Expired && !nextNote->ShouldBeRemoved && !nextNote->HasBeenEvaluated())
					{
						if (nextNoteRemainingTime < HitThresholds::ThresholdMiss)
						{
							nextNote->Expired = true;
						}
						else
						{
							i32 threshold = static_cast<i32>(note.ElapsedTime * 1000.0f);
							if (MathExtensions::IsInRange<i32>(0, 16, threshold % 100))
							{
								note.CurrentScoreBonus += 10;
							}

							hud.SetScoreBonusDisplayState(note.CurrentScoreBonus, note.TargetPosition);
						}
					}
				}

				if (remainingTime <= -MaxNoteLifetime && !note.HasBeenEvaluated())
				{
					note.ShouldBeRemoved = true;
					nextNote->ShouldBeRemoved = true;
				}
				else if (note.HasBeenEvaluated())
				{
					if (nextNoteRemainingTime < HitThresholds::ThresholdMiss && !nextNote->HasBeenEvaluated())
					{
						nextNote->Expired = true;
					}

					if (nextNoteRemainingTime <= -MaxNoteLifetime)
					{
						note.ShouldBeRemoved = true;
						nextNote->ShouldBeRemoved = true;
					}
				}
			}

			float iconProgress = 1.0f - note.GetNormalizedElapsedTime();
			note.IconPosition = MathExtensions::GetSinePoint(iconProgress, note.TargetPosition, note.EntryAngle, note.Frequency, note.Amplitude, note.Distance);
		}

		void UpdateInputBinding(NoteShape shape, const KeyBind& binding)
		{
			bool primTapped = false;
			bool altTapped = false;
			bool tapped = Keyboard::IsAnyTapped(binding, &primTapped, &altTapped);
			bool released = Keyboard::IsAnyReleased(binding, nullptr, nullptr);

			if (!tapped && !released)
			{
				return;
			}

			GameNote* note = FindNextNoteToEvaluate();

			if (note == nullptr)
			{
				note = FindNextHoldNoteToEvaluate();
			}

			if (note != nullptr)
			{
				float remainingTimeOnHit = note->GetRemainingTime() * 1000.0f;
				bool shapeMatches = note->Shape == shape;
				bool doubleGiveBonus = false;

				switch (note->Type)
				{
				case NoteType::Normal:
					note->HasBeenHit = tapped;
					if (note->HasBeenHit)
					{
						AudioEngine::GetInstance()->PlaySound(HitSound_Normal, 0.35f);
					}
					break;
				case NoteType::Double:
					note->HasBeenHitPrimary = note->HasBeenHitPrimary ? note->HasBeenHitPrimary : primTapped;
					note->HasBeenHitAlternative = note->HasBeenHitAlternative ? note->HasBeenHitAlternative : altTapped;

					// NOTE: Hold transfer implementation
					if (note->HasBeenHitPrimary && !note->HasBeenHitAlternative)
					{
						Keyboard::IsAnyDown(binding, nullptr, &note->HasBeenHitAlternative);
						note->HasBeenHit = note->HasBeenHitPrimary && note->HasBeenHitAlternative;
						AudioEngine::GetInstance()->PlaySound(HitSound_Double, 0.35f);
						break;
					}
					else if (!note->HasBeenHitPrimary && note->HasBeenHitAlternative)
					{
						Keyboard::IsAnyDown(binding, &note->HasBeenHitPrimary, nullptr);
						note->HasBeenHit = note->HasBeenHitPrimary && note->HasBeenHitAlternative;
						AudioEngine::GetInstance()->PlaySound(HitSound_Double, 0.35f);
						break;
					}

					note->HasBeenHit = note->HasBeenHitPrimary && note->HasBeenHitAlternative;
					doubleGiveBonus = note->HasBeenHit && shapeMatches;

					if (note->HasBeenHit)
					{
						AudioEngine::GetInstance()->PlaySound(HitSound_Double, 0.35f);
					}

					break;
				case NoteType::HoldStart:
					note->HasBeenHit = note->HasBeenHit ? note->HasBeenHit : tapped;
					note->CurrentScoreBonus = 10;

					hud.HoldScoreBonus();
					hud.SetScoreBonusDisplayState(note->CurrentScoreBonus, note->IconPosition);

					if (note->HasBeenHit)
					{
						HitSound_Hold_LoopVoice.SetFramePosition(0);
						HitSound_Hold_LoopVoice.SetPlaying(true);
					}

					if (note->HasBeenHit && note->NextNote->ElapsedTime < 0.0f && released)
					{
						note->NextNote->HasBeenHit = note->HasBeenHit;

						note->HitEvaluation = HitEvaluation::Miss;
						note->NextNote->HitEvaluation = HitEvaluation::Miss;
						hud.ReleaseScoreBonus(true);
						HitSound_Hold_LoopVoice.SetPlaying(false);

						note->NextNote->ShouldBeRemoved = true;
					}

					break;
				case NoteType::HoldEnd:
					note->HasBeenHit = released;

					// NOTE: Note was released early
					if (released && remainingTimeOnHit > HitThresholds::ThresholdStart)
					{
						note->HitEvaluation = HitEvaluation::Miss;
						hud.ReleaseScoreBonus(true);
						HitSound_Hold_LoopVoice.SetPlaying(false);
						break;
					}

					MainGameContext.Score.Score += note->CurrentScoreBonus;
					hud.ReleaseScoreBonus(false);

					if (note->HasBeenHit)
					{
						HitSound_Hold_LoopVoice.SetPlaying(false);
						AudioEngine::GetInstance()->PlaySound(HitSound_Hold_LoopEnd, 0.35f);
					}

					break;
				}

				if (note->HasBeenHit)
				{
					note->RemainingTimeOnHit = remainingTimeOnHit;

					if (remainingTimeOnHit <= HitThresholds::CoolThreshold && remainingTimeOnHit >= -HitThresholds::CoolThreshold)
					{
						note->HitEvaluation = HitEvaluation::Cool;
						note->HitWrong = !shapeMatches;
						MainGameContext.Score.Score += shapeMatches ? ScoreValues::Cool : ScoreValues::CoolWrong;
						MainGameContext.Score.Combo = shapeMatches ? (MainGameContext.Score.Combo + 1) : 0;
					}
					else if (remainingTimeOnHit <= HitThresholds::GoodThreshold && remainingTimeOnHit >= -HitThresholds::GoodThreshold)
					{
						note->HitEvaluation = HitEvaluation::Good;
						note->HitWrong = !shapeMatches;
						MainGameContext.Score.Score += shapeMatches ? ScoreValues::Good : ScoreValues::GoodWrong;
						MainGameContext.Score.Combo = shapeMatches ? (MainGameContext.Score.Combo + 1) : 0;
					}
					else if (remainingTimeOnHit <= HitThresholds::SafeThreshold && remainingTimeOnHit >= -HitThresholds::SafeThreshold)
					{
						note->HitEvaluation = HitEvaluation::Safe;
						note->HitWrong = !shapeMatches;
						MainGameContext.Score.Score += shapeMatches ? ScoreValues::Safe : ScoreValues::SafeWrong;
						MainGameContext.Score.Combo = 0;
					}
					else if(remainingTimeOnHit <= HitThresholds::BadThreshold && remainingTimeOnHit >= -HitThresholds::BadThreshold)
					{
						note->HitEvaluation = HitEvaluation::Bad;
						note->HitWrong = !shapeMatches;
						MainGameContext.Score.Score += shapeMatches ? ScoreValues::Bad : ScoreValues::BadWrong;
						MainGameContext.Score.Combo = 0;
					}
					else
					{
						note->HitEvaluation = HitEvaluation::Miss;
						MainGameContext.Score.Combo = 0;
					}

					hud.SetComboDisplayState(note->HitEvaluation, MainGameContext.Score.Combo, note->TargetPosition);
					if (doubleGiveBonus)
					{
						MainGameContext.Score.Score += ScoreValues::DoubleBonus;
						hud.SetScoreBonusDisplayState(200, note->TargetPosition);
					}
				}
			}
			else if (tapped && !released)
			{
				AudioEngine::GetInstance()->PlaySound(HitSound_Normal, 0.5f);
			}
		}

		GameNote* FindNextNoteToEvaluate()
		{
			for (auto& note : ActiveNotes)
			{
				if (note.HasBeenHit || note.HasBeenEvaluated() || note.Expired || note.ShouldBeRemoved || note.ElapsedTime < 0.0f)
				{
					continue;
				}

				if (note.Type == NoteType::HoldStart || note.Type == NoteType::HoldEnd)
				{
					continue;
				}

				float remainingTime = note.GetRemainingTime() * 1000.0f;
				if (remainingTime <= HitThresholds::ThresholdStart && remainingTime >= HitThresholds::ThresholdMiss)
				{
					return &note;
				}
			}

			return nullptr;
		}

		GameNote* FindNextHoldNoteToEvaluate()
		{
			for (auto& note : ActiveNotes)
			{
				if (note.Type != NoteType::HoldStart && note.Type != NoteType::HoldEnd)
				{
					continue;
				}

				if (note.Expired || note.ShouldBeRemoved || note.ElapsedTime < 0.0f)
				{
					continue;
				}

				float remainingTime = note.GetRemainingTime() * 1000.0f;
				if (note.Type == NoteType::HoldStart)
				{
					if (note.HasBeenEvaluated() && note.NextNote->HasBeenEvaluated())
					{
						continue;
					}
					else if (note.HasBeenEvaluated() && note.NextNote->ElapsedTime >= 0.0f && !note.NextNote->HasBeenEvaluated())
					{
						return note.NextNote;
					}
					else if (remainingTime <= HitThresholds::ThresholdStart && remainingTime >= HitThresholds::ThresholdMiss)
					{
						return &note;
					}
				}

				if (note.Type == NoteType::HoldEnd && note.HasBeenEvaluated())
				{
					continue;
				}
			}

			return nullptr;
		}

		void DrawNote(GameNote& note)
		{
			if (note.Expired || (note.HasBeenHit && note.Type != NoteType::HoldStart) || note.ShouldBeRemoved || note.ElapsedTime < 0.0f)
			{
				return;
			}

			bool holdStartHit = note.HasBeenHit && note.Type == NoteType::HoldStart;
			if (holdStartHit && note.NextNote->HasBeenHit == true)
			{
				return;
			}

			if (note.Type == NoteType::HoldStart)
			{
				float trailOffset = 1.0f - note.GetNormalizedElapsedTime();
				float trailLength = 0.0f;

				trailOffset = SDL_max(trailOffset, 0.0f);

				if (note.NextNote->ElapsedTime >= 0.0f)
				{
					trailLength = 1.0f - note.NextNote->GetNormalizedElapsedTime() - trailOffset;
				}
				else
				{
					trailLength = 1.0f - trailOffset;
				}

				vec2 curPoint = MathExtensions::GetSinePoint(trailOffset, note.TargetPosition, note.EntryAngle, note.Frequency, note.Amplitude, note.Distance);
				vec2 prevPoint{ curPoint };

				const Sprite* trailSprite = spriteCache.HoldNoteTrails[static_cast<size_t>(note.Shape)];
				Texture* trailTexture = iconSet.GetTexture(trailSprite->TextureIndex);

				size_t colorIndex = 0;
				for (float f = 0.0f; f <= trailLength; f += TrailResolution)
				{
					float percentage = trailOffset + f;
					curPoint = MathExtensions::GetSinePoint(percentage, note.TargetPosition, note.EntryAngle, note.Frequency, note.Amplitude, note.Distance);

					spriteRenderer->SetSpritePosition(curPoint);

					spriteRenderer->SetSpriteColor(Color {DefaultColors::White});

					float segmentLength = glm::distance(curPoint, prevPoint);
					spriteRenderer->SetSpriteScale(vec2{ segmentLength, trailSprite->SourceRectangle.Height });
					spriteRenderer->SetSpriteOrigin(vec2{ segmentLength / 2.0f, trailSprite->Origin.y });

					vec2 diff = { curPoint.y - prevPoint.y, curPoint.x - prevPoint.x };
					float angle = glm::atan(diff.x, diff.y);

					spriteRenderer->SetSpriteRotation(angle);
					spriteRenderer->SetSpriteSource(trailTexture, trailSprite->SourceRectangle);
					spriteRenderer->PushSprite(trailTexture);

					prevPoint = curPoint;
				}
			}

			switch (note.Type)
			{
			case NoteType::Normal:
				spriteRenderer->SpriteSheet().PushSprite(
					iconSet, 
					*spriteCache.NoteTargets[static_cast<size_t>(note.Shape)], 
					note.TargetPosition, 
					vec2(1.0f),
					DefaultColors::White);
				break;
			case NoteType::Double:
				spriteRenderer->SpriteSheet().PushSprite(
					iconSet,
					*spriteCache.DoubleNoteTargets[static_cast<size_t>(note.Shape)],
					note.TargetPosition,
					vec2(1.0f),
					DefaultColors::White);
				break;
			case NoteType::HoldEnd:
				spriteRenderer->SpriteSheet().PushSprite(
					iconSet,
					*spriteCache.HoldNoteTargets[static_cast<size_t>(note.Shape)],
					note.TargetPosition,
					vec2(1.0f),
					DefaultColors::White);
				break;
			}

			if (note.Type == NoteType::HoldStart && note.NextNote->ElapsedTime < 0.0f)
			{
				spriteRenderer->SpriteSheet().PushSprite(
					iconSet,
					*spriteCache.HoldNoteTargets[static_cast<size_t>(note.Shape)],
					note.TargetPosition,
					vec2(1.0f),
					DefaultColors::White);
			}

			float targetHandRotation = MathExtensions::ConvertRange(0.0f, note.NoteTime, 0.0f, MathExtensions::TwoPi, note.ElapsedTime);
			spriteRenderer->SetSpriteRotation(holdStartHit ? 0.0f : targetHandRotation);

			switch (note.Type)
			{
			default:
				spriteRenderer->SpriteSheet().PushSprite(
					iconSet,
					*spriteCache.NoteTargetHand,
					note.TargetPosition,
					vec2(1.0f),
					DefaultColors::White);
				break;
			case NoteType::Double:
				spriteRenderer->SpriteSheet().PushSprite(
					iconSet,
					*spriteCache.DoubleNoteTargetHands[static_cast<size_t>(note.Shape)],
					note.TargetPosition,
					vec2(1.0f),
					DefaultColors::White);
				break;
			}

			if (note.Type != NoteType::HoldStart && note.Type != NoteType::HoldEnd)
			{
				float trailOffset = 1.0f - note.GetNormalizedElapsedTime();
				float trailPixelLength = (note.Distance / 1000.0f) * (120.0f / note.NoteTime * 2.55f);
				float trailLength = trailPixelLength / note.Distance;

				const Sprite* trailSprite = spriteCache.Trail_Normal;
				Color trailColor = NoteTrailColors[static_cast<size_t>(note.Shape)];
				DrawNoteTrail(note, trailSprite, trailColor, false, trailOffset, trailLength, 20.0f);
			}

			//spriteRenderer->SetSpritePosition(note.IconPosition);
			//spriteRenderer->SetSpriteColor(DefaultColors::White);
			switch (note.Type)
			{
			case NoteType::Normal:
				spriteRenderer->SpriteSheet().PushSprite(
					iconSet,
					*spriteCache.NoteIcons[static_cast<size_t>(note.Shape)],
					note.IconPosition,
					vec2(1.0f),
					DefaultColors::White);
				break;
			case NoteType::Double:
				spriteRenderer->SpriteSheet().PushSprite(
					iconSet,
					*spriteCache.DoubleNoteIcons[static_cast<size_t>(note.Shape)],
					note.IconPosition,
					vec2(1.0f),
					DefaultColors::White);
				break;
			case NoteType::HoldStart:
				if (!holdStartHit)
				{
					spriteRenderer->SpriteSheet().PushSprite(
						iconSet,
						*spriteCache.HoldNoteIcons[static_cast<size_t>(note.Shape)],
						note.IconPosition,
						vec2(1.0f),
						DefaultColors::White);
				}
				break;
			case NoteType::HoldEnd:
				spriteRenderer->SpriteSheet().PushSprite(
					iconSet,
					*spriteCache.HoldNoteIcons[static_cast<size_t>(note.Shape)],
					note.IconPosition,
					vec2(1.0f),
					DefaultColors::White);
				break;
			}
		}

		void DrawNoteTrail(const GameNote& note, const Sprite* sprite, Color& color, bool hold, float offset, float length, float thickness)
		{
			vec2 curPoint = MathExtensions::GetSinePoint(offset, note.TargetPosition, note.EntryAngle, note.Frequency, note.Amplitude, note.Distance);
			vec2 prevPoint{ curPoint };

			Color curColor = color;
			Color prevColor = color;

			float trailSpriteWidth = 2.0f / length;
			float trailSpriteOffset = TrailScrollOffset;

			float trailAlpha = 0.5f;
			float trailFadeThreshold = length / 2.0f;

			Texture* trailTexture = iconSet.GetTexture(sprite->TextureIndex);

			for (float f = 0.0f; f <= length; f += TrailResolution)
			{
				float percentage = offset + f;
				curPoint = MathExtensions::GetSinePoint(percentage, note.TargetPosition, note.EntryAngle, note.Frequency, note.Amplitude, note.Distance);

				spriteRenderer->SetSpritePosition(curPoint);

				if (f >= trailFadeThreshold)
				{
					trailAlpha = MathExtensions::ConvertRange(trailFadeThreshold, length, 0.5f, 0.0f, f + TrailResolution);
					trailAlpha = SDL_max(trailAlpha, 0.0f);
				}

				prevColor = curColor;
				curColor.A = static_cast<u8>(trailAlpha * 255.0f);
				spriteRenderer->SetSpriteColors(
					prevColor, curColor,
					prevColor, curColor);

				float segmentLength = glm::distance(curPoint, prevPoint);
				spriteRenderer->SetSpriteScale(vec2{ segmentLength, thickness });
				spriteRenderer->SetSpriteOrigin(vec2{ segmentLength / 2.0f, thickness / 2.0f });

				vec2 diff = { curPoint.y - prevPoint.y, curPoint.x - prevPoint.x };
				float angle = glm::atan(diff.x, diff.y);

				spriteRenderer->SetSpriteRotation(angle);

				spriteRenderer->SetSpriteSource(trailTexture, RectangleF 
					{ trailSpriteOffset + sprite->SourceRectangle.X, 
					sprite->SourceRectangle.Y, 
					trailSpriteWidth, 
					sprite->SourceRectangle.Height });

				spriteRenderer->PushSprite(trailTexture);

				prevPoint = curPoint;
				trailSpriteOffset += trailSpriteWidth;
			}
		}
	};

	MainGameState::MainGameState()
	{
		stateInternal = new MainGameState::StateInternal(context);
		//stateInternal->MainGameContext = context;
	}
	
	bool MainGameState::Initialize()
	{
		stateInternal->Initialize();
		return true;
	}
	
	bool MainGameState::LoadContent()
	{
		return stateInternal->LoadContent();
	}
	
	void MainGameState::UnloadContent()
	{
		stateInternal->UnloadContent();
	}
	
	void MainGameState::Destroy()
	{
		stateInternal->Destroy();
		delete stateInternal;
	}
	
	void MainGameState::Update(f64 deltaTime_milliseconds)
	{
		stateInternal->Update(static_cast<float>(deltaTime_milliseconds));
	}
	
	void MainGameState::Draw(f64 deltaTime_milliseconds)
	{
		stateInternal->Draw(static_cast<float>(deltaTime_milliseconds));
	}

	std::string_view MainGameState::GetStateName() const
	{
		return "Main Game";
	}
}
