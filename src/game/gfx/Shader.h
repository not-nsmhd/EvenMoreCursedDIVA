#pragma once
#include "Resource.h"
#include <string>
#include <string_view>

namespace Starshine::GFX
{
	enum class ShaderType : u8
	{
		Vertex,
		Fragment,

		Count
	};

	constexpr EnumStringMappingTable<ShaderType> ShaderTypeStrings =
	{
		EnumStringMapping<ShaderType>
		{ShaderType::Vertex, "Vertex"},
		{ShaderType::Fragment, "Fragment"},
	};

	enum class ShaderVariableType : u8
	{
		Float,
		Vector2,
		Vector3,
		Vector4,
		Matrix4,

		Count
	};

	constexpr EnumStringMappingTable<ShaderVariableType> ShaderVariableTypeStrings =
	{
		EnumStringMapping<ShaderVariableType>
		{ShaderVariableType::Float, "Float"},
		{ShaderVariableType::Vector2, "Vector2"},
		{ShaderVariableType::Vector3, "Vector3"},
		{ShaderVariableType::Vector4, "Vector4"},
		{ShaderVariableType::Matrix4, "Matrix4"}
	};

	struct ShaderVariable
	{
		std::string Name{};

		ShaderVariableType Type{};
		void* Value = nullptr;

		ShaderType LocationShader{};
		u32 LocationIndex{};
	};

	using ShaderVariableIndex = u16;
	constexpr ShaderVariableIndex InvalidShaderVariable = 0xFFFF;

	struct Shader : public Resource
	{
	public:
		Shader(ResourceHandle handle) : Resource(ResourceType::Shader, handle) {}

		// NOTE: This function is intended for use in rendering backends only
		virtual void AddVariable(ShaderVariable& variable) = 0;

		virtual ShaderVariableIndex GetVariableIndex(std::string_view name) = 0;
		virtual void SetVariableValue(ShaderVariableIndex varIndex, void* value) = 0;
	};
}
