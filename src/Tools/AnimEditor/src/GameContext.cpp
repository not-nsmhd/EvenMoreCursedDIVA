#include "GameContext.h"
#include <IO/Path/Directory.h>
#include <IO/FileStream.h>
#include <IO/FileSystem/FileSystem.h>

using namespace Starshine;
using namespace Starshine::IO;
using namespace Starshine::Rendering;

std::unique_ptr<GameContext> GlobalInstance{};

namespace Detail
{
	bool ReadFont(Graphics::Font* font, std::string_view filePath)
	{
		IO::FileStream fileStream;
		if (!FileSystem::GetInstance()->OpenFileStream(fileStream, filePath, true, false))
			return false;

		StreamReader reader(fileStream);
		return font->ReadBinary(reader);
	}
}

GameContext::GameContext()
{
}

GameContext::~GameContext()
{
	Unload();
}

bool GameContext::Load()
{
	if (!LoadGraphics()) { return false; }
	return true;
}

void GameContext::Unload()
{
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
	DebugFont = std::make_unique<Graphics::Font>();
	if (!Detail::ReadFont(DebugFont.get(), "fonts/debug.dat")) { return false; }

	return true;
}
