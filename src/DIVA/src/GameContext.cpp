#include "GameContext.h"
#include <Common/Logging/Logging.h>
#include <IO/Path/Directory.h>
#include <IO/Path/File.h>
#include <IO/FileSystem/FileSystem.h>
#include <Rendering/Utilities.h>

using namespace Starshine;
using namespace Starshine::IO;
using namespace Starshine::Graphics;
using namespace Starshine::Rendering;
using namespace DIVA::Formats;

namespace DIVA
{
	namespace Detail
	{
		bool ReadFont(Font* font, std::string_view filePath)
		{
			FileStream fileStream;
			if (!FileSystem::GetInstance()->OpenFileStream(fileStream, filePath, true, false))
				return false;

			StreamReader reader(fileStream);
			return font->ReadBinary(reader);
		}
	}

	std::unique_ptr<GameContext> GlobalInstance{};

	GameContext::GameContext()
	{
	}

	GameContext::~GameContext()
	{
	}

	bool GameContext::Load()
	{
		if (!LoadGraphics()) { return false; }
		if (!LoadSongList()) { return false; }

		return true;
	}

	void GameContext::Unload()
	{		
		SongList.clear();
		DebugFont = nullptr;
		DefaultFont = nullptr;
		SpriteRenderer = nullptr;
	}

	bool GameContext::CreateInstance()
	{
		std::unique_ptr<GameContext> instance = std::make_unique<GameContext>();
		if (!instance->Load()) { return false; }

		GlobalInstance = std::move(instance);
		return true;
	}

	void GameContext::DestroyInstance()
	{
		GlobalInstance = nullptr;
	}

	GameContext* GameContext::GetInstance()
	{
		return GlobalInstance.get();
	}

	bool GameContext::LoadGraphics()
	{
		auto gfxDevice = Rendering::GetDevice();

		SpriteRenderer = std::make_unique<Render2D::SpriteRenderer>();

		DebugFont = std::make_unique<Font>();
		if (!Detail::ReadFont(DebugFont.get(), "fonts/debug.dat")) { return false; }

		DefaultFont = std::make_unique<Font>();
		if (!Detail::ReadFont(DefaultFont.get(), "fonts/sourcehansans.dat")) { return false; }

		Utilities::UploadTextureToGPU(DebugFont->TextureImage.get());
		Utilities::UploadTextureToGPU(DefaultFont->TextureImage.get());
			
		return true;
	}

	bool GameContext::LoadSongList()
	{
		std::string songdataDir = FileSystem::GetInstance()->GetFullDirectoryPath("songdata");
		if (songdataDir.empty())
			return false;

		Directory::IterateFiles(songdataDir, [&](std::string_view filePath)
			{
				SongInfo info{};
				if (info.ParseFromFile(filePath))
					SongList.push_back(info);
			});

		return true;
	}
}
