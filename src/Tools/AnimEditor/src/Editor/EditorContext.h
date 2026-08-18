#pragma once
#include <Common/Types.h>
#include <Graphics/AnimationSet.h>

namespace Starshine
{
	struct LayerReference
	{
		std::vector<Graphics::Layer>* Array{};
		i32 Index{};

		inline Graphics::Layer* operator->()
		{
			return &Array->at(Index);
		}

		inline const Graphics::Layer* operator->() const
		{
			return &Array->at(Index);
		}
	};

	struct EditorLayer
	{
		LayerReference BaseLayer{}; // vector reallocation strikes again
		Graphics::Transform2D CurrentTransform{};
		bool Expanded{};

		inline EditorLayer()
		{
		}

		inline EditorLayer(std::vector<Graphics::Layer>& layerList, i32 index)
		{
			BaseLayer = LayerReference{ &layerList, index };
			CurrentTransform = layerList[index].GetTransform(0.0f);
		}

		inline Graphics::Layer* GetLayerPointer()
		{
			return &BaseLayer.Array->at(BaseLayer.Index);
		}

		inline const Graphics::Layer* GetLayerPointer() const
		{
			return &BaseLayer.Array->at(BaseLayer.Index);
		}
	};

	struct EditorContextData
	{
		Graphics::AnimationSet AnimSet;
		std::vector<EditorLayer> Layers;

		Graphics::Animation* CurrentAnimation{};

		EditorLayer* CurrentLayer{};
		i32 CurrentLayerIndex{ -1 };

		f32 TimelineFrame{};

		void RecreateLayerList();
	};
}
