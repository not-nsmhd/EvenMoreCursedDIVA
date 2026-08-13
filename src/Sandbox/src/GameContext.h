#pragma once
#include <Common/Types.h>
#include <Rendering/Render2D/SpriteRenderer.h>
#include <Graphics/Font.h>
#include <vector>

namespace Sandbox
{
	class GameContext : public NonCopyable
	{
	public:
		GameContext();
		~GameContext();

	public:
		bool Load();
		void Unload();

	public:
		std::unique_ptr<Starshine::Graphics::Font> DebugFont;
		std::unique_ptr<Starshine::Rendering::Render2D::SpriteRenderer> SpriteRenderer;

	public:
		static bool CreateInstance();
		static void DestroyInstance();

		static GameContext* GetInstance();

	private:
		bool LoadGraphics();
	};
}
