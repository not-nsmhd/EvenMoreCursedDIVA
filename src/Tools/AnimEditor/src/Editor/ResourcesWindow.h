#pragma once
#include <Common/Types.h>
#include <Graphics/AnimationSet.h>
#include "EditorContext.h"

namespace Starshine
{
	class ResourcesWindow : NonCopyable
	{
	public:
		ResourcesWindow(EditorContextData* context);
		~ResourcesWindow() = default;

	public:
		void OnGUI();

	public:
		bool DrawWindow{};

	private:
		void PrepareModalWindowData();
		void NewAnimationModalWindow();
		void NewSpriteDefinitionModalWindow();

	private:
		EditorContextData* context{};

		char newName_common[128]{};
		i32 newSize_common[2]{ 0, 60 };
	};
}
