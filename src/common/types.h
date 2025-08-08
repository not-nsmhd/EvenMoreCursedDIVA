#pragma once
#include <assert.h>
#include <stdint.h>
#include <glm/glm.hpp>
#include <string>

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using vec2 = glm::vec<2, f32, glm::defaultp>;
using vec3 = glm::vec<3, f32, glm::defaultp>;
using vec4 = glm::vec<4, f32, glm::defaultp>;

using u8vec2 = glm::vec<2, u8, glm::defaultp>;
using u8vec3 = glm::vec<3, u8, glm::defaultp>;
using u8vec4 = glm::vec<4, u8, glm::defaultp>;

using mat4 = glm::mat<4, 4, f32, glm::defaultp>;

namespace DIVA
{
	// NOTE: This assumes the enum class EnumType follows the { ..., Count }; convention
	template <typename EnumType>
	constexpr size_t EnumCount()
	{
		static_assert(std::is_enum_v<EnumType>);
		return static_cast<size_t>(EnumType::Count);
	}

	template <typename Enum, typename MValue>
	struct EnumValueMapping
	{
		Enum EnumValue;
		MValue MappedValue;
	};

	template <typename Enum, typename MValue>
	using EnumValueMappingTable = EnumValueMapping<Enum, MValue>[EnumCount<Enum>()];

	template <typename Enum>
	struct EnumStringMapping
	{
		Enum EnumValue;
		std::string_view StringValue;
	};

	template <typename Enum>
	using EnumStringMappingTable = EnumStringMapping<Enum>[EnumCount<Enum>()];

	template <typename Enum>
	constexpr std::string_view EnumToString(const EnumStringMappingTable<Enum>& mapTable, Enum value)
	{
		const size_t index = static_cast<size_t>(value);
		const size_t count = EnumCount<Enum>();
		return (index < count) ? mapTable[index].StringValue : "";
	}

	template <typename Enum>
	constexpr Enum EnumFromString(const EnumStringMappingTable<Enum>& mapTable, std::string_view str)
	{
		constexpr size_t enumCount = EnumCount<Enum>();
		for (size_t i = 0; i < enumCount; i++)
		{
			if (!mapTable[i].StringValue.empty() && mapTable[i].StringValue == str)
			{
				return static_cast<Enum>(i);
			}
		}
		// TODO: There must be a better way to do this without causing a program crash or returning the first value in the enum
		return static_cast<Enum>(0);
	}
}
