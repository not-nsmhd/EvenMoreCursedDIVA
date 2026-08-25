#include "Bzip2Compression.h"
#include <Common/MathExt.h>
#include <Common/Logging/Logging.h>
#include <IO/Path/Path.h>
#include <IO/Path/File.h>
#include <Misc/Compression.h>

using namespace Starshine;
using namespace Starshine::IO;
using namespace Starshine::Misc;
using Starshine::CLI::CommandLineParser;
using Starshine::CLI::CommandLineOptionDesc;

namespace DataBuild
{
	CLI::ParseResult ParseCommandLine(int argc, char* argv[], CompressionProps& props, std::string_view defaultExtension)
	{
		constexpr CommandLineOptionDesc inputOption("--input", "-i", "Input file path");
		constexpr CommandLineOptionDesc outputOption("--output", "-o", "Output file path");

		CommandLineParser::AddOption(inputOption);
		CommandLineParser::AddOption(outputOption);

		if (CommandLineParser::Parse(argc, argv) == CLI::ParseResult::Exit)
			return CLI::ParseResult::Exit;

		if (!CommandLineParser::TryGetValue(inputOption, props.InputPath))
		{
			LogMessage("Error: Input file path was not specifed");
			return CLI::ParseResult::Exit;
		}

		if (!CommandLineParser::TryGetValue(outputOption, props.OutputPath))
			props.OutputPath = IO::Path::ChangeExtension(props.InputPath, defaultExtension);

		return CLI::ParseResult::Continue;
	}

	CLI::ParseResult CompressFile(int argc, char* argv[])
	{
		CompressionProps props{};
		
		if (ParseCommandLine(argc, argv, props, ".bz2") == CLI::ParseResult::Exit)
			return CLI::ParseResult::Exit;

		std::unique_ptr<u8[]> fileData{};
		size_t fileSize = File::ReadAllBytes(props.InputPath, fileData);

		FileStream outputStream = File::CreateWrite(props.OutputPath);
		Compression::CompressData(fileData.get(), fileSize, outputStream);

		return CLI::ParseResult::Exit;
	}

	CLI::ParseResult DecompressFile(int argc, char* argv[])
	{
		CompressionProps props{};

		if (ParseCommandLine(argc, argv, props, ".dec") == CLI::ParseResult::Exit)
			return CLI::ParseResult::Exit;

		FileStream inputStream = File::OpenRead(props.InputPath);
		FileStream outputStream = File::CreateWrite(props.OutputPath);

		Compression::DecompressStream(inputStream, inputStream.GetSize(), outputStream);

		return CLI::ParseResult::Exit;
	}
}
