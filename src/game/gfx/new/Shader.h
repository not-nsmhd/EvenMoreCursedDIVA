#pragma once
#include "Resource.h"
#include <string>
#include <string_view>

namespace Starshine::GFX
{
	enum class ShaderType
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

	enum class ShaderVariableType
	{
		Matrix4,

		Count
	};

	constexpr EnumStringMappingTable<ShaderVariableType> ShaderVariableTypeStrings =
	{
		EnumStringMapping<ShaderVariableType>
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
		Shader(void* vsData, size_t vsSize, void* fsData, size_t fsSize) 
			: Resource(ResourceType::Shader), 
			VertexShaderSource(vsData), VertexShaderSize(vsSize),
			FragmentShaderSource(fsData), FragmentShaderSize(fsSize) {}

		void* VertexShaderSource = nullptr;
		size_t VertexShaderSize = 0;

		void* FragmentShaderSource = nullptr;
		size_t FragmentShaderSize = 0;

		virtual ShaderVariableIndex GetVariableIndex(std::string_view name) = 0;
		virtual void SetVariableValue(ShaderVariableIndex varIndex, void* value) = 0;
	};
}
