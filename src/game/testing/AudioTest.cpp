#include "AudioTest.h"
#include "audio/AudioEngine.h"
#include "common/color.h"
#include "common/math_ext.h"
#include "gfx/Renderer.h"
#include "gfx/Render2D/SpriteRenderer.h"
#include "input/Keyboard.h"
#include "io/File.h"
#include "util/logging.h"

namespace Starshine::Testing
{
	using namespace Starshine::Audio;
	using namespace Starshine::GFX;
	using namespace Starshine::GFX::Render2D;
	using namespace Starshine::Input;
	using namespace Starshine::IO;
	using namespace Common;
	using std::string;
	using std::string_view;

	struct AudioTest::Impl
	{
		Renderer* BaseRenderer = nullptr;

		SpriteRenderer* SpriteRenderer = nullptr;
		Font TestFont;

		SourceHandle testAudio{};
		Voice testVoice{};

		Voice testLoopingVoice{};
		SourceHandle testLoopingAudio_start{};
		SourceHandle testLoopingAudio_end{};

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

			testAudio = AudioEngine::GetInstance()->LoadSource("diva/sounds/test1.wav");
			testVoice = AudioEngine::GetInstance()->AllocateVoice(testAudio);

			testLoopingAudio_start = AudioEngine::GetInstance()->LoadSource("diva/sounds/test_loop.wav");
			testLoopingAudio_end = AudioEngine::GetInstance()->LoadSource("diva/sounds/test_loop_end.wav");
			testLoopingVoice = AudioEngine::GetInstance()->AllocateVoice(testLoopingAudio_start);

			return true;
		}

		void Destroy()
		{
			TestFont.Destroy();
			SpriteRenderer->Destroy();
		}

		void Update(f64 deltaTime_milliseconds)
		{
			if (Keyboard::IsKeyTapped(SDLK_SPACE))
			{
				testVoice.SetFramePosition(0);
				testVoice.SetPlaying(true);
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
				testLoopingVoice.SetSource(testLoopingAudio_end);
				testLoopingVoice.SetLoopState(false);
				testLoopingVoice.SetFramePosition(0);
				testLoopingVoice.SetPlaying(true);
			}

			SDL_memset(debugText, 0, sizeof(debugText));

			int pos = SDL_snprintf(debugText, sizeof(debugText) - 1, "\n\n(Press spacebar to play a test sound)");
			pos += SDL_snprintf(debugText + pos, sizeof(debugText) - 1, "\n(Hold the H key to test audio looping)");

			pos += SDL_snprintf(debugText + pos, sizeof(debugText) - 1, "\n\nVoice Position: %llu", testVoice.GetFramePosition());
			pos += SDL_snprintf(debugText + pos, sizeof(debugText) - 1, "\nLooping Voice Position: %llu", testLoopingVoice.GetFramePosition());
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
