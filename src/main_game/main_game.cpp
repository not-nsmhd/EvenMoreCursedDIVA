#include "global_res.h"
#include "audio/helpers.h"
#include "gfx/helpers/tex_helpers.h"
#include "main_game.h"
#include "common/math_ext.h"

#include <fstream>

using namespace Common;
using std::fstream;
using std::ios;
using std::string;

namespace MainGame
{
	static GFX::Font* debugFont;

	static const string l_ShapeNames[] = 
	{
		"Triangle", 	// NOTE_TRIANGLE
		"Circle", 		// NOTE_CIRCLE
		"Cross", 		// NOTE_CROSS
		"Square", 		// NOTE_SQUARE
		"Star" 			// NOTE_STAR
	};

	static const string l_TargetSpritePrefix = "Target_";
	static const string l_IconSpritePrefix = "Icon_";
	static const string l_TargetHandSprite = "TargetHand";

	MainGameState::MainGameState()
	{
	}

	MainGameState* MainGameState::instance = new MainGameState();
	
	MainGameState* MainGameState::GetInstance()
	{
		return instance;
	}
	
	bool MainGameState::Initialize()
	{
		noteArea_ScaleFactor = vec2(
			static_cast<float>(game->windowWidth) / noteArea_BaseSize.x,
			static_cast<float>(game->windowHeight) / noteArea_BaseSize.y
			);

		return true;
	}
	
	bool MainGameState::LoadContent()
	{
		spriteRenderer.Initialize(graphicsBackend);
		debugFont = GlobalResources::DebugFont;
		iconSet.ReadFromTextFile(graphicsBackend, "sprites/iconset_ps3");

		for (size_t i = 0; i < static_cast<int>(NoteShape::NOTE_SHAPE_COUNT); i++)
		{
			cachedNoteTargetSprites[i] = iconSet.GetSprite(l_TargetSpritePrefix + 
				l_ShapeNames[(i % static_cast<int>(NoteShape::NOTE_SHAPE_COUNT))]);

			cachedNoteIconSprites[i] = iconSet.GetSprite(l_IconSpritePrefix + 
				l_ShapeNames[(i % static_cast<int>(NoteShape::NOTE_SHAPE_COUNT))]);
		}
		noteTargetHandSprite = iconSet.GetSprite(l_TargetHandSprite);

		bgTexture = GFX::Helpers::LoadImage(graphicsBackend, "sprites/game_bg.png");
		
		Audio::Helpers::LoadSoundEffect(&hitSE, "sounds/test_pcm.wav");
		hitSE.Volume = 0.182f;

		songMusic.LoadFromFile("music/pv_032.ogg");

		songChart.LoadFromXml("songdata/test/test_chart2.xml");

		return true;
	}
	
	void MainGameState::UnloadContent()
	{
		spriteRenderer.Destroy();
		iconSet.Destroy();
		graphicsBackend->DestroyTexture(bgTexture);

		audio->StopAllSFXVoices();
		hitSE.Destroy();
		audio->StopAllStreamingVoices();
		songMusic.Destroy();

		songChart.Clear();
	}
	
	void MainGameState::Destroy()
	{
		elapsedTime = 0.0f;
		over = false;
		paused = false;
		manualUpdate = false;
		noteWrong = false;
		noteHitPos = {};

		noteValu.clear();
		gameScore.Reset();

		autoPlay = false;
		musicStarted = false;

		activeNotes.clear();
		chartNoteOffset = 0;
		chartEventOffset = 0;
	}
	
	void MainGameState::OnResize(u32 newWidth, u32 newHeight)
	{
	}
	
	void MainGameState::Update()
	{
		if (!manualUpdate && !paused && !over)
		{
			gameStep();
		}

		if (!autoPlay && !paused && !over)
		{
			handleNoteInput();
		}

		handleDebugInput();
		updateDebug();
	}
	
	void MainGameState::Draw()
	{
		graphicsBackend->Clear(GFX::LowLevel::ClearFlags::GFX_CLEAR_COLOR, Common::Color(0, 24, 24, 255), 1.0f, 0);
		graphicsBackend->SetBlendState(&GFX::LowLevel::DefaultBlendStates::AlphaBlend);

		spriteRenderer.SetSpritePosition(vec2(0.0f, 0.0f));
		spriteRenderer.SetSpriteScale(vec2(game->windowWidth, game->windowHeight));
		spriteRenderer.SetSpriteSource(bgTexture, Common::RectangleF(0.0f, 0.0f, 640.0f, 360.0f));
		spriteRenderer.SetSpriteColor(Common::Color(156, 156, 156, 255));
		spriteRenderer.PushSprite(bgTexture);

		for (std::deque<GameNote>::iterator note = activeNotes.begin(); note != activeNotes.end(); note++)
		{
			if (note->state != GameNoteState::ACTIVE)
			{
				continue;
			}

			ChartNote* cNote = note->noteStats;

			// Note Target
			spriteRenderer.SetSpritePosition(cNote->Position * noteArea_ScaleFactor);
			spriteRenderer.SetSpriteColor(DefaultColors::White);
			iconSet.PushSprite(spriteRenderer, cachedNoteTargetSprites[static_cast<int>(cNote->Shape)], noteArea_ScaleFactor);

			// Timing Bar
			spriteRenderer.SetSpritePosition(cNote->Position * noteArea_ScaleFactor);
			spriteRenderer.SetSpriteColor(DefaultColors::White);

			float normalizedNoteTime = MathExtensions::ConvertRange(0.0f, note->flyTime_seconds, 0.0f, 1.0f, note->elapsedTime_seconds);
			spriteRenderer.SetSpriteRotation(normalizedNoteTime * MathExtensions::MATH_EXT_2PI);

			iconSet.PushSprite(spriteRenderer, noteTargetHandSprite, noteArea_ScaleFactor);

			// Note Icon
			vec2 iconPos = MathExtensions::GetSinePoint(1.0f - normalizedNoteTime, cNote->Position, cNote->Angle,
				cNote->Frequency, cNote->Amplitude, cNote->Distance);

			spriteRenderer.SetSpritePosition(iconPos * noteArea_ScaleFactor);
			spriteRenderer.SetSpriteColor(DefaultColors::White);
			iconSet.PushSprite(spriteRenderer, cachedNoteIconSprites[static_cast<int>(cNote->Shape)], noteArea_ScaleFactor);
		}

		// debug display
		drawDebug();
		spriteRenderer.RenderSprites(nullptr);

		graphicsBackend->SwapBuffers();
	}
	
	void MainGameState::gameStep()
	{
		elapsedTime += game->deltaTime_ms / 1000.0f;
		float deltaTime_seconds = game->deltaTime_ms / 1000.0f;

		for (std::deque<GameNote>::iterator note = activeNotes.begin(); note != activeNotes.end(); note++)
		{
			/*note->Update(game->deltaTime_ms);

			if (note->HasExpired() || note->HasBeenHit() || note->HasBeenWrongHit())
			{
				activeNotes.pop_front();
				break;
			}

			if (autoPlay)
			{
				if (note->GetRemainingTime() <= 0.0f)
				{
					inputNoteHit(note->GetShape(), false, true);
				}
			}*/

			if (autoPlay && note->elapsedTime_seconds + deltaTime_seconds >= note->flyTime_seconds)
			{
				inputNoteHit(note->noteStats->Shape, false, true);
			}

			if (note->state == GameNoteState::EXPIRED || note->state == GameNoteState::HIT)
			{
				activeNotes.pop_front();
				continue;
			}

			note->elapsedTime_seconds += deltaTime_seconds;

			if (note->elapsedTime_seconds >= note->flyTime_seconds + 0.13f)
			{
				note->state = GameNoteState::EXPIRED;
				continue;
			}
		}

		for (size_t i = chartNoteOffset; i < songChart.Notes.size(); i++)
		{
			ChartNote* cNote = &songChart.Notes[i];
			if (cNote->AppearTime <= elapsedTime)
			{
				GameNote gNote = {};

				gNote.flyTime_seconds = currentNoteDuration_seconds;
				gNote.noteStats = cNote;
				gNote.state = GameNoteState::ACTIVE;

				activeNotes.push_back(gNote);
				chartNoteOffset++;
			}
		}

		for (std::vector<ChartEvent*>::const_iterator event = songChart.Events.cbegin() + chartEventOffset; event!= songChart.Events.cend(); event++)
		{
			if (elapsedTime >= (*event)->ExecutionTime)
			{
				switch ((*event)->Type)
				{
					case ChartEventType::EVENT_SET_BPM:
						{
							const SetBPMEvent* eventPtr = static_cast<const SetBPMEvent*>(*event);
							currentNoteDuration_seconds = MathExtensions::CalculateBarDuration_Seconds(eventPtr->BPM, eventPtr->BeatsPerBar);
							break;
						}
					case ChartEventType::EVENT_SONG_END:
						{
							over = true;
							break;
						}
					case ChartEventType::EVENT_PLAY_MUSIC:
						{
							audio->PlayMusic(&songMusic);
							break;
						}
				}

				chartEventOffset++;
			}
		}
	}
	
	void MainGameState::inputNoteHit(NoteShape shape, bool secondary, bool down)
	{
		if (!activeNotes.empty())
		{
			GameNote* firstHittableNote = nullptr;

			size_t i = 0;
			while (true)
			{
				GameNote* gNote = &activeNotes[i];
				i++;

				if (gNote->state == GameNoteState::ACTIVE)
				{
					firstHittableNote = gNote;
					break;
				}
			}

			if (firstHittableNote != nullptr)
			{
				float remainingTime_ms = (firstHittableNote->flyTime_seconds - firstHittableNote->elapsedTime_seconds) * 1000.0f;
				if (!MathExtensions::IsInRange(-130.0f, 130.0f, remainingTime_ms))
				{
					return;
				}
				else
				{
					firstHittableNote->state = GameNoteState::HIT;
					HitValuation valu = HitValuation::NONE;

					if (MathExtensions::IsInRange(-30.0f, 30.0f, remainingTime_ms))
					{
						valu = HitValuation::COOL;
					}
					else if (MathExtensions::IsInRange(-70.0f, 70.0f, remainingTime_ms))
					{
						valu = HitValuation::GOOD;
					}
					else if (MathExtensions::IsInRange(-100.0f, 100.0f, remainingTime_ms))
					{
						valu = HitValuation::SAFE;
					}
					else
					{
						valu = HitValuation::BAD;
					}

					noteHitPos = firstHittableNote->noteStats->Position;
					noteWrong = shape != firstHittableNote->noteStats->Shape;
					gameScore.RegisterNoteHit(valu, noteWrong);
					noteValu = HitValuationNames[static_cast<int>(valu)];
				}
			}	
		}

		audio->PlaySoundEffect(hitSE);
	}
	
	void MainGameState::handleNoteInput()
	{
		if (keyboardState->IsKeyTapped(SDL_SCANCODE_D))
		{
			inputNoteHit(NoteShape::NOTE_CIRCLE, false, true);
		}

		if (keyboardState->IsKeyTapped(SDL_SCANCODE_L))
		{
			inputNoteHit(NoteShape::NOTE_CIRCLE, false, true);
		}

		if (keyboardState->IsKeyTapped(SDL_SCANCODE_S))
		{
			inputNoteHit(NoteShape::NOTE_CROSS, false, true);
		}

		if (keyboardState->IsKeyTapped(SDL_SCANCODE_K))
		{
			inputNoteHit(NoteShape::NOTE_CROSS, false, true);
		}

		if (keyboardState->IsKeyTapped(SDL_SCANCODE_A))
		{
			inputNoteHit(NoteShape::NOTE_SQUARE, false, true);
		}

		if (keyboardState->IsKeyTapped(SDL_SCANCODE_J))
		{
			inputNoteHit(NoteShape::NOTE_SQUARE, false, true);
		}

		if (keyboardState->IsKeyTapped(SDL_SCANCODE_W))
		{
			inputNoteHit(NoteShape::NOTE_TRIANGLE, false, true);
		}

		if (keyboardState->IsKeyTapped(SDL_SCANCODE_I))
		{
			inputNoteHit(NoteShape::NOTE_TRIANGLE, false, true);
		}
	}
	
	void MainGameState::handleDebugInput()
	{
		if (keyboardState->IsKeyTapped(SDL_SCANCODE_PERIOD))
		{
			manualUpdate = true;
			gameStep();
		}

		if (keyboardState->IsKeyTapped(SDL_SCANCODE_R))
		{
			manualUpdate = false;
		}

		if (keyboardState->IsKeyDown(SDL_SCANCODE_LSHIFT))
		{
			if (keyboardState->IsKeyTapped(SDL_SCANCODE_L))
			{
				autoPlay = !autoPlay;
			}
		}

		if (keyboardState->IsKeyTapped(SDL_SCANCODE_ESCAPE))
		{
			paused = !paused;
		}
	}
	
	void MainGameState::updateDebug()
	{
		SDL_memset(debugStateString, 0, sizeof(debugStateString));

		int pos = 0;
		pos += SDL_snprintf(debugStateString + pos, sizeof(debugStateString) - 1, "Elapsed Time: %.3f\n", elapsedTime);
		pos += SDL_snprintf(debugStateString + pos, sizeof(debugStateString) - 1, "Chart: %d/%d\n", chartNoteOffset, songChart.Notes.size());
		pos += SDL_snprintf(debugStateString + pos, sizeof(debugStateString) - 1, "Events: %d/%d\n", chartEventOffset, songChart.Events.size());
		pos += SDL_snprintf(debugStateString + pos, sizeof(debugStateString) - 1, "Active Notes: %d\n", activeNotes.size());
		pos += SDL_snprintf(debugStateString + pos, sizeof(debugStateString) - 1, "Autoplay (Shift+L): %s\n", autoPlay ? "True" : "False");

		if (manualUpdate)
		{
			pos += SDL_snprintf(debugStateString + pos, sizeof(debugStateString) - 1, "\nMANUAL UPDATE\n" \
			"Press '.' key to timestep\nPress 'R' key to resume");
		}
	}
	
	void MainGameState::drawDebug()
	{
		debugFont->PushString(spriteRenderer, debugStateString, sizeof(debugStateString) - 1, vec2(4.0f), vec2(1.0f), Common::DefaultColors::White);
		debugFont->PushString(spriteRenderer, noteValu, noteHitPos * noteArea_ScaleFactor, vec2(1.0f), 
		noteWrong ? Common::DefaultColors::Red : Common::DefaultColors::White);

		debugFont->PushString(spriteRenderer, std::to_string(gameScore.GetCurrentScore()), glm::vec2(1150.0f, 48.0f), glm::vec2(1.0f), Common::DefaultColors::White);
		debugFont->PushString(spriteRenderer, std::to_string(gameScore.GetCurrentCombo()), glm::vec2(1150.0f, 64.0f), glm::vec2(1.0f), Common::DefaultColors::White);

		if (paused)
		{
			spriteRenderer.SetSpriteColor(Common::Color(0, 0, 0, 128));
			spriteRenderer.SetSpriteScale(vec2(game->windowWidth, game->windowHeight));
			spriteRenderer.PushSprite(nullptr);

			debugFont->PushString(spriteRenderer, "Paused", vec2(game->windowWidth / 2.0f, game->windowHeight / 2.0f), vec2(1.0f), Common::DefaultColors::White);
		}
	}
}
