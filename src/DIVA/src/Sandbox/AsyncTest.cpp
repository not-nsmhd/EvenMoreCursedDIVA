#include "AsyncTest.h"
#include "../GameContext.h"
#include "../Definitions.h"
#include <Input/Keyboard.h>
#include <future>
#include <IO/FileStream.h>
#include <IO/StreamReader.h>
#include <IO/FileSystem/FileSystem.h>

using namespace Starshine;
using namespace Starshine::Graphics;
using namespace Starshine::Input;
using namespace Starshine::IO;
using Starshine::Rendering::Render2D::SpriteRenderer;

namespace DIVA::Sandbox
{
	struct AsyncTest::Impl
	{
		struct fileEntry
		{
			std::string Name;

			u16 X{};
			u16 Y{};
			u16 Width{};
			u16 Height{};

			f32 OriginX{};
			f32 OriginY{};
		};

		std::vector<fileEntry> entries;
		bool done = false;

		f32 loadSprRotation{};
		std::future<void> taskFuture;

		i32 entryIndex{};

		void AsyncLoadFunc()
		{
			FileStream fileStream;
			if (!FileSystem::GetInstance()->OpenFileStream(fileStream, "testfiles/DataTest.dat", true, false))
				return;

			SDL_Delay(5000);
			StreamReader reader = StreamReader(fileStream);

			char signature[4]{};
			reader.ReadBuffer(signature, 4);
			i32 entryCount = reader.ReadI32();
			reader.SkipUntilAligned(16);

			entries.reserve(entryCount);
			for (i32 i = 0; i < entryCount; i++)
			{
				fileEntry& newEntry = entries.emplace_back();
				newEntry.Name = reader.ReadStringPointer();

				newEntry.X = reader.ReadU16();
				newEntry.Y = reader.ReadU16();
				newEntry.Width = reader.ReadU16();
				newEntry.Height = reader.ReadU16();

				newEntry.OriginX = reader.ReadF32();
				newEntry.OriginY = reader.ReadF32();
			}

			fileStream.Close();

			entryIndex = 0;
			done = true;
		}

		Impl()
		{
		}

		void Init()
		{
			taskFuture = std::async(std::launch::async, [&]{ AsyncLoadFunc(); });
		}

		void Update(GameTime& gameTime)
		{
			if (!done)
			{
				static constexpr f64 baseFrameTime = 1.0 / 60.0;
				loadSprRotation += gameTime.ElapsedFrameTime.GetSeconds() / baseFrameTime;
			}
			else
			{
				if (Keyboard::IsKeyTapped(SDLK_RIGHT))
					entryIndex = MathExtensions::Clamp<i32>(entryIndex + 1, 0, entries.size() - 1);
				if (Keyboard::IsKeyTapped(SDLK_LEFT))
					entryIndex = MathExtensions::Clamp<i32>(entryIndex - 1, 0, entries.size() - 1);
			}
		}

		void Draw()
		{
			using namespace Starshine::Rendering;

			auto gfxDevice = Rendering::GetDevice();
			auto sprRenderer = GameContext::GetInstance()->SpriteRenderer.get();
			auto& fontRenderer = sprRenderer->Font();
			auto font = GameContext::GetInstance()->DebugFont.get();

			gfxDevice->Clear(ClearFlags_Color, DefaultColors::ClearColor_Menus, 1.0f, 0);

			fontRenderer.PushString(font, "Async Test", {}, vec2(1.0f), DefaultColors::White, DefaultColors::Black);

			if (done)
			{
				char text[92]{};
				SDL_snprintf(text, sizeof(text), "entries: %d/%llu", entryIndex + 1, entries.size());
				fontRenderer.PushString(font, text, vec2(128.0f), vec2(1.0f), DefaultColors::White, DefaultColors::Black);

				const fileEntry& currentEntry = entries[entryIndex];
				SDL_snprintf(text, sizeof(text), "\nentry %d: %s { %d %d %d %d } { %f %f }",
					entryIndex, currentEntry.Name.c_str(),
					currentEntry.X, currentEntry.Y, currentEntry.Width, currentEntry.Height,
					currentEntry.OriginX, currentEntry.OriginY);
				fontRenderer.PushString(font, text, vec2(128.0f), vec2(1.0f), DefaultColors::White, DefaultColors::Black);
			}
			else
			{
				sprRenderer->SetSpritePosition(vec2(128.0f, 128.0f));
				sprRenderer->SetSpriteSize(vec2(64.0f));
				sprRenderer->SetSpriteOrigin(vec2(32.0f));
				sprRenderer->SetSpriteRotation(MathExtensions::ToRadians(loadSprRotation));
				sprRenderer->PushSprite(nullptr);

				fontRenderer.PushString(font, "Loading", vec2(128.0f), vec2(1.0f), DefaultColors::White, DefaultColors::Black);
			}
			
			sprRenderer->Present();
		}
	};

	AsyncTest::AsyncTest()
	{
	}

	AsyncTest::~AsyncTest()
	{
	}

	bool AsyncTest::Initialize()
	{
		GameInstance->GetWindow()->SetTitle("Testing");
		impl = std::make_unique<Impl>();
		impl->Init();
		return true;
	}

	bool AsyncTest::LoadContent()
	{
		return true;
	}

	void AsyncTest::UnloadContent()
	{
	}

	void AsyncTest::Destroy()
	{
		GameInstance->GetWindow()->SetTitle("Even More Cursed DIVA");
		impl = nullptr;
	}

	void AsyncTest::Update(Starshine::GameTime& gameTime)
	{
		impl->Update(gameTime);

		if (Keyboard::IsKeyTapped(SDLK_ESCAPE))
			GameInstance->SetState(GetStatePointer(StateID::ChartSelect));
	}

	void AsyncTest::Draw(Starshine::GameTime& gameTime)
	{
		impl->Draw();
	}
}
