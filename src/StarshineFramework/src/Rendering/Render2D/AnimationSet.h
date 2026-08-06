#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "Common/Types.h"
#include "SpriteSheet.h"
#include "Common.h"

namespace Starshine::Rendering::Render2D
{
	template <typename T>
	struct Keyframe
	{
		u32 Frame{};
		T Value{};

		Keyframe() {};
		Keyframe(const u32& frame, const T& value) : Frame(frame), Value(value) {};
	};

	template <typename T>
	bool InterpolateKeyframes(const std::vector<Keyframe<T>>& keyframes, const f32& frame, T& value)
	{
		if (keyframes.empty())
			return false;

		if (keyframes.size() == 1 || frame <= 0.0f)
		{
			value = keyframes[0].Value;
			return true;
		}

		const Keyframe<T>& first = keyframes.front();
		const Keyframe<T>& last = keyframes.back();

		if (frame <= first.Frame)
		{
			value = first.Value;
			return true;
		}

		if (frame >= last.Frame)
		{
			value = last.Value;
			return true;
		}

		const Keyframe<T>* start = &keyframes[0];
		const Keyframe<T>* end = start;

		for (size_t i = 0; i < keyframes.size(); i++)
		{
			end = &keyframes[i];
			if (end->Frame >= frame)
				break;

			start = end;
		}

		const f32 range = static_cast<f32>(end->Frame - start->Frame);
		const f32 f = frame - static_cast<f32>(start->Frame);
		const f32 frameFactor = MathExtensions::ConvertRange<f32>(0.0f, range, 0.0f, 1.0f, f);
		value = start->Value * (1.0f - frameFactor) + end->Value * frameFactor;

		return true;
	}

	struct SpriteDefinition
	{
		std::string Name;
		vec2 Size{};

		const Sprite* RealSprite{};
	};

	struct Layer
	{
	public:
		Transform2D GetTransform(const f32& frame) const;

	public:
		std::string Name;

		u32 StartTime{};
		u32 EndTime{};

		BlendMode BlendMode{ BlendMode::Normal };

		std::vector<Keyframe<vec2>> Origin;
		std::vector<Keyframe<vec2>> Position;
		std::vector<Keyframe<vec2>> Scale;
		std::vector<Keyframe<f32>> Rotation;
		std::vector<Keyframe<Color>> Color;

		const SpriteDefinition* SpriteDefinition{};
	};

	struct Animation
	{
	public:
		const Layer& GetLayer(std::string_view name) const;
		const Layer& GetLayer(const size_t& index) const;

	public:
		std::string Name;

		u32 StartTime{};
		u32 EndTime{};

		std::vector<Layer> Layers;
	};

	class AnimationSet
	{
	public:
		AnimationSet() = default;
		~AnimationSet() = default;

	public:
		bool ReadXml(std::string_view xmlData);
		bool LoadXml(std::string_view filePath);

		void LinkToSpriteSheet(std::shared_ptr<SpriteSheet> spriteSheet);

	public:
		const Animation& GetAnimation(const size_t& index) const;
		const Animation& GetAnimation(std::string_view name) const;
		i32 GetAnimationIndex(std::string_view name) const;

		const SpriteDefinition& GetSpriteDefinition(std::string_view name) const;

	public:
		ivec2 GetResolution() const;
		i32 GetFPS() const;

		f32 GetRelativeFrameTimeStep(const f32& frameTime) const;

	private:
		std::string name;
		std::string spriteSheetPath;

		std::shared_ptr<SpriteSheet> linkedSpriteSheet;
		std::vector<SpriteDefinition> spriteDefinitions;
		std::vector<Animation> animations;

		ivec2 resolution{};
		i32 fps{ 60 };
	};
}
