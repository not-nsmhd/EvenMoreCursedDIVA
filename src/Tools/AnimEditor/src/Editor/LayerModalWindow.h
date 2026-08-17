#pragma once
#include <Common/Types.h>
#include "EditorContext.h"

namespace Starshine
{
	class LayerModalWindow : NonCopyable
	{
	public:
		LayerModalWindow(EditorContextData* context);
		~LayerModalWindow() = default;

	public:
		void OpenRename(i32 layerIndex);
		void OpenChangeSprite(i32 layerIndex);
		void OpenRemoveLayer(i32 layerIndex);

		void OnGUI();

	private:
		void RenameLayerWindow();
		void ChangeLayerSpriteWindow();
		void RemoveLayerWindow();

		EditorContextData* context{};

		Graphics::Layer* layerToModify{};

		char newLayerName[128]{};
		Graphics::SpriteDefinition* spriteToSet{};

		i32 layerToRemoveIndex{ -1 };
	};
}
