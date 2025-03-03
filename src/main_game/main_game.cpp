#include "global_res.h"
#include "main_game.h"
#include "common/math_ext.h"

#include <fstream>

using namespace Common;
using std::fstream;
using std::ios;

namespace MainGame
{
	static GFX::Font* debugFont;

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
		elapsedTime = 0.0f;
		manualUpdate = false;
		noteWrong = false;
		noteHitPos = {};

		noteValu.clear();
		gameScore.Reset();

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

		songChart.LoadFromXml("songdata/test/test_chart.xml");

		return true;
	}
	
	void MainGameState::UnloadContent()
	{
		spriteRenderer.Destroy();
		iconSet.Destroy();
	}
	
	void MainGameState::Destroy()
	{
		activeNotes.clear();
		songChart.Clear();
		chartNoteOffset = 0;
	}
	
	void MainGameState::OnResize(u32 newWidth, u32 newHeight)
	{
	}
	
	void MainGameState::Update()
	{
		if (!manualUpdate)
		{
			gameStep();
		}

		handleNoteInput();
		handleDebugInput();

		updateDebug();
	}
	
	void MainGameState::Draw()
	{
		graphicsBackend->Clear(GFX::LowLevel::ClearFlags::GFX_CLEAR_COLOR, Common::Color(0, 24, 24, 255), 1.0f, 0);
		graphicsBackend->SetBlendState(&GFX::LowLevel::DefaultBlendStates::AlphaBlend);

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

		for (std::vector<ChartNote>::const_iterator note = songChart.Notes.cbegin() + chartNoteOffset; note != songChart.Notes.cend(); note++)
		{
			ChartNote notePtr = *note; 
			if (elapsedTime >= note->AppearTime)
			{
				activeNotes.push_back(Note(&iconSet, 1.333f, &notePtr, noteArea_ScaleFactor));
				chartNoteOffset++;
			}
		}

		for (std::deque<Note>::iterator note = activeNotes.begin(); note != activeNotes.end(); note++)
		{
			note->Update(game->deltaTime_ms);
		}
	}
	
	void MainGameState::inputNoteHit(NoteShape shape, bool secondary, bool down)
	{
		if (activeNotes.size() < 0)
		{
			return;
		}

		Note* firstHittableNote = nullptr;
		int firstHittableNoteIndex = 0;

		Note* testNote = nullptr;
		for (int i = 0; i < activeNotes.size(); i++)
		{
			testNote = &activeNotes[i];

			if (!testNote->HasBeenWrongHit() && !testNote->HasBeenHit() && !testNote->HasExpired())
			{
				firstHittableNote = testNote;
				firstHittableNoteIndex = i;
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
				activeNotes.erase(activeNotes.cbegin() + firstHittableNoteIndex);
			}
		}
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
	}
	
	void MainGameState::updateDebug()
	{
		SDL_memset(debugStateString, 0, sizeof(debugStateString));

		int pos = 0;
		pos += SDL_snprintf(debugStateString + pos, sizeof(debugStateString) - 1, "Elapsed Time: %.3f\n", elapsedTime);
		pos += SDL_snprintf(debugStateString + pos, sizeof(debugStateString) - 1, "Chart: %d/%d\n", chartNoteOffset, songChart.Notes.size());
		pos += SDL_snprintf(debugStateString + pos, sizeof(debugStateString) - 1, "Active Notes: %d\n", activeNotes.size());

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
	}
}
