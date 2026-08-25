#pragma once
#include <Common/Types.h>
#include "CommandLineParser.h"
#include <Graphics/Texture.h>
#include <Misc/ImageHelper.h>

namespace DataBuild
{
	struct TextureConversionProps
	{
		std::string InputPath{};
		std::string OutputPath{};
		Starshine::Graphics::TextureFormat Format{};

		bool WrapS{};
		bool WrapT{};
		bool NearestFilter{};

		bool CompressQOI{};
		bool CompressBZ2{};

		bool Extract{};
		Starshine::Misc::ImageHelper::ImageFormat ExtractionFormat{};
	};

	Starshine::CLI::ParseResult ParseTextureConversionCommandLine(int argc, char* argv[], TextureConversionProps& props);
	Starshine::CLI::ParseResult ConvertTexture(int argc, char* argv[]);
}
