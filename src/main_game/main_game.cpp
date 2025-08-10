#include "main_game.h"
#include "chart.h"
#include "hit_evaluation.h"
#include "hud.h"
#include "../common/types.h"
#include "../common/math_ext.h"
#include "../global_res.h"
#include "../gfx/helpers/tex_helpers.h"
#include "../gfx/sprite_renderer.h"
#include "../gfx/sprite_sheet.h"
#include "../gfx/font.h"
#include <string>
#include <deque>
#include <fstream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

namespace MainGame
{
	using namespace DIVA;
	using namespace GFX;
	using namespace Input;
	using namespace Common;
	using std::string_view;
	using std::fstream;
	using std::ios_base;

	constexpr vec2 DefaultIconPosition { std::numeric_limits<float>().infinity(), std::numeric_limits<float>().infinity() };
	constexpr float MaxNoteLifetime { 1000.0f };
	constexpr float TrailResolution { 1.0f / 64.0f };
	constexpr float TrailLengthFactor { 64.0f / 1.25f };
	constexpr float TrailScrollSpeed { 64.0f * TrailResolution };

	constexpr Color NoteTrailColors[EnumCount<NoteShape>()]
	{
		{ 237,  68,  78, 255 },
		{ 181, 255, 255, 255 },
		{ 255, 206, 255, 255 },
		{ 242, 255, 175, 255 }
	};

	// TODO: Replace this with sprites
	

	struct GameNote
	{
		float NoteTime;
		float ElapsedTime;

		float RemainingTimeOnHit;

		NoteShape Shape;
		NoteType Type;
		vec2 TargetPosition;
		vec2 IconPosition;

		float EntryAngle;
		float Frequency;
		float Amplitude;
		float Distance;

		// NOTE: Generic state flags
		bool Expired = false;
		bool HasBeenHit = false;
		bool ShouldBeRemoved = false;

		// NOTE: Double-specific state flags
		bool HasBeenHitPrimary = false;
		bool HasBeenHitAlternative = false;

		// NOTE: Used by hold notes (and possibly linked star notes if they get added)
		GameNote* NextNote = nullptr;

		// NOTE: Hit Stats
		HitEvaluation HitEvaluation;
		bool HitWrong = false;

		// NOTE: Functions
		// NOTE: Returned value is specified in seconds
		float GetRemainingTime() const { return NoteTime - ElapsedTime; }
		float GetNormalizedElapsedTime() const { return Common::MathExtensions::ConvertRange(0.0f, NoteTime, 0.0f, 1.0f, ElapsedTime); }

		bool HasBeenEvaluated() const { return HitEvaluation != HitEvaluation::None; }
	};

	struct MainGameState::StateInternal
	{
		GFXBackend* gfxBackend;
		Context& MainGameContext;

		SpriteRenderer* spriteRenderer;
		SpriteSheet iconSet;

		float TrailScrollOffset = 0.0f;
		bool Paused = false;

		struct SpriteCache
		{
			Sprite* NoteTargets[EnumCount<NoteShape>()];
			Sprite* NoteIcons[EnumCount<NoteShape>()];
			Sprite* NoteTargetHand;

			Sprite* DoubleNoteTargets[EnumCount<NoteShape>()];
			Sprite* DoubleNoteIcons[EnumCount<NoteShape>()];
			Sprite* DoubleNoteTargetHands[EnumCount<NoteShape>()];

			Sprite* HoldNoteTargets[EnumCount<NoteShape>()];
			Sprite* HoldNoteIcons[EnumCount<NoteShape>()];

			Sprite* HoldNoteTrails[EnumCount<NoteShape>()];

			Sprite* Trail_Normal;
			Sprite* Trail_CT;
		} spriteCache;

		Chart songChart;
		size_t chartNoteOffset = 0;

		float ElapsedTime_Seconds = 0.0f;
		std::deque<GameNote> ActiveNotes;

		Keyboard* keyboard = Keyboard::GetInstance();
		KeyBind AdvanceKeybind = KeyBind(keyboard, SDL_SCANCODE_PERIOD, KeyBind::UnsetScancode );
		KeyBind PauseKeybind = KeyBind(keyboard, SDL_SCANCODE_ESCAPE, KeyBind::UnsetScancode );
		EnumValueMappingTable<NoteShape, KeyBind> NoteKeybinds
		{
			EnumValueMapping<NoteShape, KeyBind> { NoteShape::Circle, KeyBind(keyboard, SDL_SCANCODE_D, SDL_SCANCODE_L) },
			EnumValueMapping<NoteShape, KeyBind> { NoteShape::Cross, KeyBind(keyboard, SDL_SCANCODE_S, SDL_SCANCODE_K) },
			EnumValueMapping<NoteShape, KeyBind> { NoteShape::Square, KeyBind(keyboard, SDL_SCANCODE_A, SDL_SCANCODE_J) },
			EnumValueMapping<NoteShape, KeyBind> { NoteShape::Triangle, KeyBind(keyboard, SDL_SCANCODE_W, SDL_SCANCODE_I) }
		};

		HUD hud = HUD(MainGameContext);

		char debugText[512] = {};

		StateInternal(MainGame::Context& context) : MainGameContext{ context }
		{
		}

		void Initialize()
		{
			hud.Initialize();
		}

		bool LoadContent()
		{
			spriteRenderer = GlobalResources::SpriteRenderer;

			// ---------------
			iconSet.ReadFromTextFile(gfxBackend, "sprites/iconset_ps3");

			spriteCache.NoteTargetHand = iconSet.GetSprite("TargetHand");
			spriteCache.Trail_Normal = iconSet.GetSprite("Trail_Normal");
			spriteCache.Trail_CT = iconSet.GetSprite("Trail_CT");

			auto fetchNoteShapeSpecificSprite = [&](NoteShape shape, std::string_view name, Sprite* spriteArray[])
			{
				spriteArray[static_cast<size_t>(shape)] = iconSet.GetSprite(name);
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
				ElapsedTime_Seconds += deltaTime_ms / 1000.0f;

				UpdateChart(deltaTime_ms);

				size_t noteIndex = 0;
				while (noteIndex < ActiveNotes.size())
				{
					GameNote* note = &ActiveNotes[noteIndex];

					if (note->ShouldBeRemoved)
					{
						ActiveNotes.erase(ActiveNotes.cbegin() + noteIndex);
						continue;
					}

					UpdateNote(*note, deltaTime_ms);
					noteIndex++;
				}

				for (size_t i = 0; i < DIVA::EnumCount<NoteShape>(); i++)
				{
					UpdateInputBinding(NoteKeybinds[i].EnumValue, NoteKeybinds[i].MappedValue);
				}

				TrailScrollOffset += TrailScrollSpeed * (16.6667f / deltaTime_ms);
				TrailScrollOffset = SDL_fmodf(TrailScrollOffset, 128.0f);

				hud.Update(deltaTime_ms);
			}

			if (PauseKeybind.IsTapped(nullptr, nullptr))
			{
				Paused = !Paused;
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
			gfxBackend->Clear(GFX::LowLevel::ClearFlags::GFX_CLEAR_COLOR, Common::Color(0, 24, 24, 255), 1.0f, 0);
			gfxBackend->SetBlendState(&GFX::LowLevel::DefaultBlendStates::AlphaBlend);

			for (auto& note : ActiveNotes)
			{
				DrawNote(note);
			}

			hud.Draw(deltaTime_ms);

			GlobalResources::DebugFont->PushString(spriteRenderer, debugText, sizeof(debugText), vec2(0.0f, 0.0f), vec2(1.0f), Common::DefaultColors::White);
			spriteRenderer->RenderSprites(nullptr);

			gfxBackend->SwapBuffers();
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
			note.ElapsedTime += deltaTime_ms / 1000.0f;

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
			bool tapped = binding.IsTapped(&primTapped, &altTapped);
			bool released = binding.IsReleased(nullptr, nullptr);

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
					break;
				case NoteType::Double:
					note->HasBeenHitPrimary = note->HasBeenHitPrimary ? note->HasBeenHitPrimary : primTapped;
					note->HasBeenHitAlternative = note->HasBeenHitAlternative ? note->HasBeenHitAlternative : altTapped;

					// NOTE: Hold transfer implementation
					if (note->HasBeenHitPrimary && !note->HasBeenHitAlternative)
					{
						binding.IsDown(nullptr, &note->HasBeenHitAlternative);
						note->HasBeenHit = note->HasBeenHitPrimary && note->HasBeenHitAlternative;
						break;
					}
					else if (!note->HasBeenHitPrimary && note->HasBeenHitAlternative)
					{
						binding.IsDown(&note->HasBeenHitPrimary, nullptr);
						note->HasBeenHit = note->HasBeenHitPrimary && note->HasBeenHitAlternative;
						break;
					}

					note->HasBeenHit = note->HasBeenHitPrimary && note->HasBeenHitAlternative;
					doubleGiveBonus = note->HasBeenHit && shapeMatches;
					break;
				case NoteType::HoldStart:
					note->HasBeenHit = note->HasBeenHit ? note->HasBeenHit : tapped;

					if (note->NextNote->ElapsedTime < 0.0f && released)
					{
						note->NextNote->HasBeenHit = note->HasBeenHit;
						note->HitEvaluation = HitEvaluation::Miss;
						note->NextNote->HitEvaluation = HitEvaluation::Miss;
						note->NextNote->ShouldBeRemoved = true;
					}

					break;
				case NoteType::HoldEnd:
					note->HasBeenHit = released;

					// NOTE: Note was released early
					if (released && remainingTimeOnHit > HitThresholds::ThresholdStart)
					{
						note->HitEvaluation = HitEvaluation::Miss;
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
						MainGameContext.Score.Combo = shapeMatches ? (MainGameContext.Score.Combo + 1) : 0;
					}
					else if (remainingTimeOnHit <= HitThresholds::GoodThreshold && remainingTimeOnHit >= -HitThresholds::GoodThreshold)
					{
						note->HitEvaluation = HitEvaluation::Good;
						note->HitWrong = !shapeMatches;
						MainGameContext.Score.Combo = shapeMatches ? (MainGameContext.Score.Combo + 1) : 0;
					}
					else if (remainingTimeOnHit <= HitThresholds::SafeThreshold && remainingTimeOnHit >= -HitThresholds::SafeThreshold)
					{
						note->HitEvaluation = HitEvaluation::Safe;
						note->HitWrong = !shapeMatches;
						MainGameContext.Score.Combo = 0;
					}
					else if(remainingTimeOnHit <= HitThresholds::BadThreshold && remainingTimeOnHit >= -HitThresholds::BadThreshold)
					{
						note->HitEvaluation = HitEvaluation::Bad;
						note->HitWrong = !shapeMatches;
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
						hud.SetScoreBonusDisplayState(200, note->TargetPosition);
					}
				}
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

				float remainingTime = note.GetRemainingTime();
				if (remainingTime <= 0.13f && remainingTime >= -0.13f)
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

				float remainingTime = note.GetRemainingTime();
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
					else if (remainingTime <= 0.13f && remainingTime >= -0.13f)
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

				Sprite* trailSprite = spriteCache.HoldNoteTrails[static_cast<size_t>(note.Shape)];
				LowLevel::Texture* trailTexture = iconSet.GetTexture(trailSprite->texIndex);

				size_t colorIndex = 0;
				for (float f = 0.0f; f <= trailLength; f += TrailResolution)
				{
					float percentage = trailOffset + f;
					curPoint = MathExtensions::GetSinePoint(percentage, note.TargetPosition, note.EntryAngle, note.Frequency, note.Amplitude, note.Distance);

					spriteRenderer->SetSpritePosition(curPoint);

					spriteRenderer->SetSpriteColor(Color {DefaultColors::White});

					float segmentLength = glm::distance(curPoint, prevPoint);
					spriteRenderer->SetSpriteScale(vec2{ segmentLength, trailSprite->sourceRect.height });
					spriteRenderer->SetSpriteOrigin(vec2{ segmentLength / 2.0f, trailSprite->origin.y });

					vec2 diff = { curPoint.y - prevPoint.y, curPoint.x - prevPoint.x };
					float angle = glm::atan(diff.x, diff.y);

					spriteRenderer->SetSpriteRotation(angle);
					spriteRenderer->SetSpriteSource(trailTexture, trailSprite->sourceRect);
					spriteRenderer->PushSprite(trailTexture);

					prevPoint = curPoint;
				}
			}

			spriteRenderer->SetSpritePosition(note.TargetPosition);
			spriteRenderer->SetSpriteColor(DefaultColors::White);
			switch (note.Type)
			{
			case NoteType::Normal:
				iconSet.PushSprite(spriteRenderer, spriteCache.NoteTargets[static_cast<size_t>(note.Shape)]);
				break;
			case NoteType::Double:
				iconSet.PushSprite(spriteRenderer, spriteCache.DoubleNoteTargets[static_cast<size_t>(note.Shape)]);
				break;
			case NoteType::HoldEnd:
				iconSet.PushSprite(spriteRenderer, spriteCache.HoldNoteTargets[static_cast<size_t>(note.Shape)]);
				break;
			}

			if (note.Type == NoteType::HoldStart && note.NextNote->ElapsedTime < 0.0f)
			{
				iconSet.PushSprite(spriteRenderer, spriteCache.HoldNoteTargets[static_cast<size_t>(note.Shape)]);
			}

			float targetHandRotation = Common::MathExtensions::ConvertRange(0.0f, note.NoteTime, 0.0f, Common::MathExtensions::MATH_EXT_2PI, note.ElapsedTime);
			spriteRenderer->SetSpritePosition(note.TargetPosition);
			spriteRenderer->SetSpriteRotation(holdStartHit ? 0.0f : targetHandRotation);
			spriteRenderer->SetSpriteColor(DefaultColors::White);
			switch (note.Type)
			{
			case NoteType::Normal:
				iconSet.PushSprite(spriteRenderer, spriteCache.NoteTargetHand);
				break;
			case NoteType::Double:
				iconSet.PushSprite(spriteRenderer, spriteCache.DoubleNoteTargetHands[static_cast<size_t>(note.Shape)]);
				break;
			case NoteType::HoldStart:
			case NoteType::HoldEnd:
				iconSet.PushSprite(spriteRenderer, spriteCache.NoteTargetHand);
				break;
			}

			if (note.Type != NoteType::HoldStart && note.Type != NoteType::HoldEnd)
			{
				float trailOffset = 1.0f - note.GetNormalizedElapsedTime();
				float trailPixelLength = (note.Distance / 1000.0f) * (120.0f / note.NoteTime * 2.55f);
				float trailLength = trailPixelLength / note.Distance;

				Sprite* trailSprite = spriteCache.Trail_Normal;
				Color trailColor = NoteTrailColors[static_cast<size_t>(note.Shape)];
				DrawNoteTrail(note, trailSprite, trailColor, false, trailOffset, trailLength, 20.0f);
			}

			spriteRenderer->SetSpritePosition(note.IconPosition);
			spriteRenderer->SetSpriteColor(DefaultColors::White);
			switch (note.Type)
			{
			case NoteType::Normal:
				iconSet.PushSprite(spriteRenderer, spriteCache.NoteIcons[static_cast<size_t>(note.Shape)]);
				break;
			case NoteType::Double:
				iconSet.PushSprite(spriteRenderer, spriteCache.DoubleNoteIcons[static_cast<size_t>(note.Shape)]);
				break;
			case NoteType::HoldStart:
				if (!holdStartHit)
				{
					iconSet.PushSprite(spriteRenderer, spriteCache.HoldNoteIcons[static_cast<size_t>(note.Shape)]);
				}
				break;
			case NoteType::HoldEnd:
				iconSet.PushSprite(spriteRenderer, spriteCache.HoldNoteIcons[static_cast<size_t>(note.Shape)]);
				break;
			}
		}

		void DrawNoteTrail(const GameNote& note, Sprite* sprite, Color& color, bool hold, float offset, float length, float thickness)
		{
			vec2 curPoint = MathExtensions::GetSinePoint(offset, note.TargetPosition, note.EntryAngle, note.Frequency, note.Amplitude, note.Distance);
			vec2 prevPoint{ curPoint };

			Color curColor = color;
			Color prevColor = color;

			float trailSpriteWidth = 2.0f / length;
			float trailSpriteOffset = TrailScrollOffset;

			float trailAlpha = 0.5f;
			float trailFadeThreshold = length / 2.0f;

			LowLevel::Texture* trailTexture = iconSet.GetTexture(sprite->texIndex);

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
					{ trailSpriteOffset + sprite->sourceRect.x, 
					sprite->sourceRect.y, 
					trailSpriteWidth, 
					sprite->sourceRect.height });

				spriteRenderer->PushSprite(trailTexture);

				prevPoint = curPoint;
				trailSpriteOffset += trailSpriteWidth;
			}
		}
	};

	MainGameState::MainGameState()
	{
		context.SpriteRenderer = GlobalResources::SpriteRenderer;

		stateInternal = new MainGameState::StateInternal(context);
		stateInternal->MainGameContext = context;
	}

	MainGameState::~MainGameState()
	{
	}
	
	bool MainGameState::Initialize()
	{
		stateInternal->gfxBackend = game->GetGraphicsBackend();
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
	
	void MainGameState::OnResize(u32 newWidth, u32 newHeight)
	{
	}
	
	void MainGameState::Update()
	{
		stateInternal->Update(static_cast<float>(game->deltaTime_ms));
	}
	
	void MainGameState::Draw()
	{
		stateInternal->Draw(static_cast<float>(game->deltaTime_ms));
	}
}
