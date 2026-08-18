#include "EditorContext.h"

namespace Starshine
{
	void EditorContextData::RecreateLayerList()
	{
		Layers.clear();
		Layers.reserve(CurrentAnimation->Layers.size());

		i32 layerIndex = 0;
		for (auto& layer : CurrentAnimation->Layers)
		{
			EditorLayer editLayer = EditorLayer(CurrentAnimation->Layers, layerIndex++);
			Layers.push_back(editLayer);
		}
	}
}
