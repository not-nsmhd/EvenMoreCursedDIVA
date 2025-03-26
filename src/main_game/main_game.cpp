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
		hitSE.Volume = 0.1f;

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

		for (std::deque<Note>::iterator note = activeNotes.begin(); note != activeNotes.end(); note++)
		{
			note->Draw(spriteRenderer);
		}

		// debug display
		drawDebug();
		spriteRenderer.RenderSprites(nullptr);

		graphicsBackend->SwapBuffers();
	}
	
	void MainGameState::gameStep()
	{
		elapsedTime += game->deltaTime_ms / 1000.0f;

		for (std::deque<Note>::iterator note = activeNotes.begin(); note != activeNotes.end(); note++)
		{
			note->Update(game->deltaTime_ms);

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
			}
		}

		for (std::vector<ChartNote>::const_iterator note = songChart.Notes.cbegin() + chartNoteOffset; note != songChart.Notes.cend(); note++)
		{
			ChartNote notePtr = *note;
			Note newNote(currentNoteDuration_seconds, &notePtr, noteArea_ScaleFactor);
			newNote.SetResources(&iconSet, cachedNoteIconSprites, cachedNoteTargetSprites, noteTargetHandSprite);
			if (elapsedTime >= note->AppearTime)
			{
				activeNotes.push_back(newNote);
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
			Note* firstHittableNote = nullptr;

			Note* testNote = nullptr;
			for (int i = 0; i < activeNotes.size(); i++)
			{
				testNote = &activeNotes[i];

				if (!testNote->HasBeenWrongHit() && !testNote->HasBeenHit() && !testNote->HasExpired())
				{
					firstHittableNote = testNote;
					break;
				}
			}

			if (firstHittableNote != nullptr)
			{
				firstHittableNote->SendInput(shape, secondary, down);

				if (firstHittableNote->HasBeenHit())
				{
					noteHitPos = firstHittableNote->GetTargetPosition();
					noteWrong = firstHittableNote->HasBeenWrongHit();
					HitValuation valu = firstHittableNote->GetHitValuation();
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
		debugFont->PushString(spriteRenderer, noteValu, noteHitPos, vec2(1.0f), 
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
