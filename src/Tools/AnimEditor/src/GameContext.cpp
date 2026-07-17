#include "GameContext.h"
#include <IO/Path/Directory.h>

using namespace Starshine;
using namespace Starshine::IO;
using namespace Starshine::Rendering;

std::unique_ptr<GameContext> GlobalInstance{};

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
	DebugFont = std::make_unique<Render2D::Font>();
	if (!DebugFont->ReadBMFont("diva/fonts/debug.fnt")) { return false; }

	return true;
}
