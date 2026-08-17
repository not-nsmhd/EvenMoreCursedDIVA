#pragma once
#include <Common/Types.h>
#include <Graphics/SpriteSheet.h>

namespace Starshine
{
	struct EditorSprite
	{
		Graphics::Sprite* BaseSprite{};

		vec2 SourceShift{};
		vec2 RealSize{};
	};

	class SpriteEditorWindow : NonCopyable
	{
	public:
		SpriteEditorWindow() = default;
		~SpriteEditorWindow() = default;

	public:
		void SetSpriteSheet(Graphics::SpriteSheet* spriteSheet);
		void SetCurrentSprite(Graphics::Sprite* sprite);
		void OnGUI();

	public:
		bool DrawWindow{};

	private:
		void WindowMenuBar();
		void ExportSpriteProperties(std::string_view dirPath);
		void ExportSpritePropertyFile(std::string_view dirPath, const EditorSprite& sprite);

	private:
		Graphics::SpriteSheet* currentSpriteSheet{};

		std::vector<EditorSprite> sprites;
		EditorSprite* currentSprite{};

		vec2 displayScale{ 1.0f, 1.0f };
	};
}
