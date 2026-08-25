#pragma once
#include <Common/Types.h>
#include "CommandLineParser.h"

namespace DataBuild
{
	struct CompressionProps
	{
		std::string InputPath{};
		std::string OutputPath{};
	};

	Starshine::CLI::ParseResult CompressFile(int argc, char* argv[]);
	Starshine::CLI::ParseResult DecompressFile(int argc, char* argv[]);
}
