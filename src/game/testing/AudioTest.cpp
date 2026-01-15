#include "AudioTest.h"
#include "audio/AudioEngine.h"
#include <Common/Types.h>
#include <Common/MathExt.h>
#include "gfx/Renderer.h"
#include "gfx/Render2D/SpriteRenderer.h"
#include "input/Keyboard.h"
#include "IO/Path/File.h"
#include "util/logging.h"

namespace Starshine::Testing
{
	using namespace Starshine::Audio;
	using namespace Starshine::GFX;
	using namespace Starshine::GFX::Render2D;
	using namespace Starshine::Input;
	using namespace Starshine::IO;
	using std::string;
	using std::string_view;

	struct AudioTest::Impl
	{
		Renderer* BaseRenderer = nullptr;

		SpriteRenderer* SpriteRenderer = nullptr;
		Font TestFont;

		SourceHandle testAudio{};

		Voice testLoopingVoice{};
		SourceHandle testLoopingAudio_start{};
		SourceHandle testLoopingAudio_end{};

		SourceHandle testStreamingAudio{};
		Voice testStreamingVoice{};

		char debugText[512] {};

		bool Initialize()
		{
			BaseRenderer = Renderer::GetInstance();

			return true;
		}

		bool LoadContent()
		{
			SpriteRenderer = new Render2D::SpriteRenderer();

			TestFont.ReadBMFont("diva/fonts/debug.fnt");

			testAudio = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Star_Normal01.ogg");

			testLoopingAudio_start = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Star_Hold01_Loop.ogg");
			testLoopingAudio_end = AudioEngine::GetInstance()->LoadSource("diva/sounds/mg_notes/Star_Hold01_LoopEnd.ogg");
			testLoopingVoice = AudioEngine::GetInstance()->AllocateVoice(testLoopingAudio_start);

			testStreamingAudio = AudioEngine::GetInstance()->LoadStreamingSource("diva/music/test.ogg");
			testStreamingVoice = AudioEngine::GetInstance()->AllocateVoice(testStreamingAudio);

			return true;
		}

		void Destroy()
		{
			AudioEngine::GetInstance()->UnloadSource(testAudio);
			AudioEngine::GetInstance()->UnloadSource(testLoopingAudio_start);
			AudioEngine::GetInstance()->UnloadSource(testLoopingAudio_end);
			AudioEngine::GetInstance()->UnloadSource(testStreamingAudio);

			TestFont.Destroy();
			SpriteRenderer->Destroy();
		}

		void Update(f64 deltaTime_milliseconds)
		{
			if (Keyboard::IsKeyTapped(SDLK_SPACE))
			{
				AudioEngine::GetInstance()->PlaySound(testAudio, 0.7f);
			}

			if (Keyboard::IsKeyTapped(SDLK_h))
			{
				testLoopingVoice.SetSource(testLoopingAudio_start);
				testLoopingVoice.SetLoopState(true);
				testLoopingVoice.SetFramePosition(0);
				testLoopingVoice.SetPlaying(true);
			}
			else if (Keyboard::IsKeyReleased(SDLK_h))
			{
				testLoopingVoice.SetFramePosition(0);
				testLoopingVoice.SetPlaying(false);

				AudioEngine::GetInstance()->PlaySound(testLoopingAudio_end, 1.0f);
			}

			if (Keyboard::IsKeyTapped(SDLK_s))
			{
				testStreamingVoice.SetLoopState(true);
				testStreamingVoice.SetPlaying(true);
			}

			SDL_memset(debugText, 0, sizeof(debugText));

			int pos = SDL_snprintf(debugText, sizeof(debugText) - 1, "\n\n(Press spacebar to play a test sound)");
			pos += SDL_snprintf(debugText + pos, sizeof(debugText) - 1, "\n(Hold the H key to test audio looping)");
			pos += SDL_snprintf(debugText + pos, sizeof(debugText) - 1, "\n(Press S to test audio streaming)");

			pos += SDL_snprintf(debugText + pos, sizeof(debugText) - 1, "\nLooping Voice Position: %llu", testLoopingVoice.GetFramePosition());
			pos += SDL_snprintf(debugText + pos, sizeof(debugText) - 1, "\nStreaming Voice Position: %llu", testStreamingVoice.GetFramePosition());
		}

		void Draw(f64 deltaTime_milliseconds)
		{
			BaseRenderer->Clear(ClearFlags_Color, Color(0, 24, 24, 255), 1.0f, 0);

			SpriteRenderer->Font().PushString(TestFont, "Audio Test", vec2(0.0f), vec2(1.0f), DefaultColors::White);
			SpriteRenderer->Font().PushString(TestFont, debugText, vec2(0.0f), vec2(1.0f), DefaultColors::White);
			SpriteRenderer->RenderSprites(nullptr);

			BaseRenderer->SwapBuffers();
		}
	};

	AudioTest::AudioTest() : impl(new AudioTest::Impl())
	{
	}

	bool AudioTest::Initialize()
	{
		return impl->Initialize();
	}

	bool AudioTest::LoadContent()
	{
		return impl->LoadContent();
	}

	void AudioTest::UnloadContent()
	{
	}

	void AudioTest::Destroy()
	{
		impl->Destroy();
		delete impl;
	}

	void AudioTest::Update(f64 deltaTime_milliseconds)
	{
		impl->Update(deltaTime_milliseconds);
	}

	void AudioTest::Draw(f64 deltaTime_milliseconds)
	{
		impl->Draw(deltaTime_milliseconds);
	}

	string_view AudioTest::GetStateName() const
	{
		return "[Dev] Audio Test";
	}
}
