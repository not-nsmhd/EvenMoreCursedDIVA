#include "AnimationSet.h"
#include "Common/MathExt.h"
#include "IO/Xml.h"
#include "IO/Path/File.h"

namespace Starshine::Rendering::Render2D
{
	namespace XmlElementNames
	{
		static constexpr const char* AnimationSet = "AnimationSet";
		static constexpr const char* AnimationSet_Width = "Width";
		static constexpr const char* AnimationSet_Height = "Height";
		static constexpr const char* AnimationSet_FPS = "FPS";
		static constexpr const char* AnimationSet_StageColor = "StageColor";
		static constexpr const char* AnimationSet_SpriteSheet = "SpriteSheet";

		static constexpr const char* SpriteDefs = "SpriteDefinitions";
		static constexpr const char* SpriteDefs_Sprite = "SpriteDefinition";

		static constexpr const char* Common_Name = "Name";
		static constexpr const char* Common_Start = "Start";
		static constexpr const char* Common_End = "End";
		static constexpr const char* Common_Sprite = "Sprite";
		static constexpr const char* Common_Size = "Size";

		static constexpr const char* Animation = "Animation";
		static constexpr const char* AnimationLayer = "Layer";
		static constexpr const char* AnimationLayer_BlendMode = "BlendMode";

		static constexpr std::string_view Keyframes_Origin = "Origin";
		static constexpr std::string_view Keyframes_Position = "Position";
		static constexpr std::string_view Keyframes_Scale = "Scale";
		static constexpr std::string_view Keyframes_Rotation = "Rotation";
		static constexpr std::string_view Keyframes_Color = "Color";

		static constexpr const char* Keyframe = "Keyframe";
		static constexpr const char* Keyframe_Frame = "Frame";
		static constexpr const char* Keyframe_Value = "Value";
	}

	namespace Detail
	{
		template <typename T>
		void ReadKeyframes_Xml(std::vector<Keyframe<T>>& keyframes, std::string_view elementName, const Xml::Element* layerElement)
		{
			const Xml::Element* frameListElement = layerElement->FirstChildElement(elementName.data());
			if (frameListElement == nullptr)
				return;

			for (const Xml::Element* frameElement = frameListElement->FirstChildElement(XmlElementNames::Keyframe);
				frameElement;
				frameElement = frameElement->NextSiblingElement(XmlElementNames::Keyframe))
			{
				i32 frame = 0;
				if (frameElement->QueryIntAttribute(XmlElementNames::Keyframe_Frame, &frame) != 0)
					return;

				T value;
				const Xml::Attribute* valueAttrib = frameElement->FindAttribute(XmlElementNames::Keyframe_Value);
				Xml::TryGetValue(value, valueAttrib);

				keyframes.emplace_back(Keyframe<T>(frame, value));
			}
		}

		void ReadKeyframes_Xml(std::vector<Keyframe<f32>>& keyframes, std::string_view elementName, const Xml::Element* layerElement)
		{
			const Xml::Element* frameListElement = layerElement->FirstChildElement(elementName.data());
			if (frameListElement == nullptr)
				return;

			for (const Xml::Element* frameElement = frameListElement->FirstChildElement(XmlElementNames::Keyframe);
				frameElement;
				frameElement = frameElement->NextSiblingElement(XmlElementNames::Keyframe))
			{
				i32 frame = 0;
				f32 value = 0.0f;

				if (frameElement->QueryIntAttribute(XmlElementNames::Keyframe_Frame, &frame) != 0)
					return;

				if (frameElement->QueryFloatAttribute(XmlElementNames::Keyframe_Value, &value) != 0)
					return;

				keyframes.emplace_back(Keyframe<f32>(frame, value));
			}
		}
	}

	Transform2D Layer::GetTransform(const f32& frame) const
	{
		if (!MathExtensions::IsInRange<u32>(StartTime, EndTime, static_cast<u32>(frame)))
			return Transform2D::Zero();

		Transform2D result;

		vec2 tempVec2{};
		Starshine::Color tempColor{};

		if (InterpolateKeyframes(Origin, frame, tempVec2))
			result.Origin = tempVec2;
		if (InterpolateKeyframes(Position, frame, tempVec2))
			result.Position = tempVec2;
		if (InterpolateKeyframes(Scale, frame, tempVec2))
			result.Scale = tempVec2;
		if (InterpolateKeyframes(Rotation, frame, tempVec2.x))
			result.Rotation = tempVec2.x;
		if (InterpolateKeyframes(Color, frame, tempColor))
			result.Color = tempColor;

		return result;
	};

	const Layer& Animation::GetLayer(std::string_view name) const
	{
		for (const auto& layer : Layers)
		{
			if (layer.Name == name)
				return layer;
		}
		return Layers[0];
	};

	const Layer& Animation::GetLayer(const size_t& index) const
	{
		return Layers.at(index);
	};

	bool AnimationSet::ReadXml(std::string_view xmlData)
	{
		if (xmlData.size() == 0)
			return false;

		Xml::Document animSetDoc;
		if (!Xml::Parse(animSetDoc, xmlData.data(), xmlData.size()))
			return false;

		const Xml::Element* rootElement = animSetDoc.FirstChildElement(XmlElementNames::AnimationSet);
		rootElement->QueryIntAttribute(XmlElementNames::AnimationSet_Width, &resolution.x);
		rootElement->QueryIntAttribute(XmlElementNames::AnimationSet_Height, &resolution.y);
		rootElement->QueryIntAttribute(XmlElementNames::AnimationSet_FPS, &fps);

#if 0
		const char* sprSheetPath{};
		if (rootElement->QueryAttribute(XmlElementNames::AnimationSet_SpriteSheet, &sprSheetPath) == 0)
		{
			spriteSheetPath = sprSheetPath;
			ImportSpritesFromFolder(sprSheetPath);
		}
#endif

		const Xml::Element* sprDefsElement = rootElement->FirstChildElement(XmlElementNames::SpriteDefs);

		for (const Xml::Element* sprDefElement = sprDefsElement->FirstChildElement(XmlElementNames::SpriteDefs_Sprite);
			sprDefElement;
			sprDefElement = sprDefElement->NextSiblingElement(XmlElementNames::SpriteDefs_Sprite))
		{
			auto& sprDef = spriteDefinitions.emplace_back();

			const char* elementName = sprDefElement->Attribute(XmlElementNames::Common_Name);
			sprDef.Name = elementName;

			Xml::TryGetValue(sprDef.Size, Xml::FindAttribute(sprDefElement, XmlElementNames::Common_Size));
		}

		for (const Xml::Element* animElement = rootElement->FirstChildElement(XmlElementNames::Animation);
			animElement;
			animElement = animElement->NextSiblingElement(XmlElementNames::Animation))
		{
			auto& anim = animations.emplace_back();

			const char* animName{};
			if (animElement->QueryAttribute(XmlElementNames::Common_Name, &animName) == 0)
				anim.Name = animName;

			animElement->QueryUnsignedAttribute(XmlElementNames::Common_Start, &anim.StartTime);
			animElement->QueryUnsignedAttribute(XmlElementNames::Common_End, &anim.EndTime);

			for (const Xml::Element* layerElement = animElement->FirstChildElement(XmlElementNames::AnimationLayer);
				layerElement;
				layerElement = layerElement->NextSiblingElement(XmlElementNames::AnimationLayer))
			{
				Layer& layer = anim.Layers.emplace_back();

				const char* elementName = layerElement->Attribute(XmlElementNames::Common_Name);
				layer.Name = std::string(elementName);

				const char* refName{};
				if (layerElement->QueryAttribute(XmlElementNames::Common_Sprite, &refName) == 0)
					layer.SpriteDefinition = &GetSpriteDefinition(refName);

				// TODO: Implement animation referencing

				const char* blendModeName{};
				if (layerElement->QueryAttribute(XmlElementNames::AnimationLayer_BlendMode, &blendModeName) == 0)
				{
					for (size_t i = 0; i < EnumCount<BlendMode>(); i++)
					{
						if (BlendModeNames[i] == blendModeName)
						{
							layer.BlendMode = static_cast<BlendMode>(i);
							break;
						}
					}
				}

				layerElement->QueryUnsignedAttribute(XmlElementNames::Common_Start, &layer.StartTime);
				layerElement->QueryUnsignedAttribute(XmlElementNames::Common_End, &layer.EndTime);

				Detail::ReadKeyframes_Xml(layer.Origin, XmlElementNames::Keyframes_Origin, layerElement);
				Detail::ReadKeyframes_Xml(layer.Position, XmlElementNames::Keyframes_Position, layerElement);
				Detail::ReadKeyframes_Xml(layer.Scale, XmlElementNames::Keyframes_Scale, layerElement);
				Detail::ReadKeyframes_Xml(layer.Rotation, XmlElementNames::Keyframes_Rotation, layerElement);
				Detail::ReadKeyframes_Xml(layer.Color, XmlElementNames::Keyframes_Color, layerElement);
			}
		}

		return true;
	}

	bool AnimationSet::LoadXml(std::string_view filePath)
	{
		std::unique_ptr<char[]> xmlData;
		size_t xmlSize = IO::File::ReadAllText(filePath, xmlData);

		if (xmlData == nullptr || xmlSize == 0)
		{
			return false;
		}

		bool result = ReadXml(std::string_view(xmlData.get(), xmlSize));
		return result;
	}

	void AnimationSet::LinkToSpriteSheet(std::shared_ptr<SpriteSheet> spriteSheet)
	{
		for (auto& sprDef : spriteDefinitions)
		{
			const i32 sprIndex = spriteSheet->GetSpriteIndex(sprDef.Name);

			if (sprIndex != -1)
				sprDef.RealSprite = &spriteSheet->GetSprite(sprIndex);
		}

		linkedSpriteSheet = spriteSheet;
	}

	ivec2 AnimationSet::GetResolution() const
	{
		return resolution;
	}

	i32 AnimationSet::GetFPS() const
	{
		return fps;
	}

	f32 AnimationSet::GetRelativeFrameTimeStep(const f32& frameTime) const
	{
		if (fps > 0)
		{
			const f32 animFrameTime = 1.0f / static_cast<f32>(fps);
			return frameTime / animFrameTime;
		}

		return 1.0f;
	}
 
	const Animation& AnimationSet::GetAnimation(const size_t& index) const
	{
		if (MathExtensions::IsInRange<size_t>(0, animations.size() - 1, index))
			return animations[index];

		return animations[0];
	}

	const Animation& AnimationSet::GetAnimation(std::string_view name) const
	{
		for (const auto& anim : animations)
		{
			if (anim.Name == name)
				return anim;
		}
		return animations[0];
	}

	i32 AnimationSet::GetAnimationIndex(std::string_view name) const
	{
		i32 index = 0;
		for (const auto& anim : animations)
		{
			if (anim.Name == name)
				return index;

			index++;
		}
		return -1;
	}

	const SpriteDefinition& AnimationSet::GetSpriteDefinition(std::string_view name) const
	{
		for (const auto& sprDef : spriteDefinitions)
		{
			if (sprDef.Name == name)
				return sprDef;
		}
		return spriteDefinitions[0];
	}
}
