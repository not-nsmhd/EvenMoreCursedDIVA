#include "FontConverter.h"
#include <Common/Logging/Logging.h>
#include <IO/Path/Path.h>
#include <IO/Path/File.h>
#include <Graphics/Font.h>
#include "TextureConverter.h"

using namespace Starshine;
using namespace Starshine::IO;
using namespace Starshine::Graphics;
using namespace Starshine::Misc;
using namespace Starshine::Misc::ImageHelper;
using Starshine::CLI::CommandLineParser;
using Starshine::CLI::CommandLineOptionDesc;

namespace DataBuild
{
	struct FontConversionProps
	{
		std::string InputPath{};
		std::string OutputPath{};

		TextureConversionProps TextureProps{};
	};

	CLI::ParseResult ParseCommandLine(int argc, char* argv[], FontConversionProps& props)
	{
		constexpr CommandLineOptionDesc inputOption("--input", "-i", "Input file path");
		constexpr CommandLineOptionDesc outputOption("--output", "-o", "Output file path");

		CommandLineParser::AddOption(inputOption);
		CommandLineParser::AddOption(outputOption);

		if (ParseTextureConversionCommandLine(argc, argv, props.TextureProps) == CLI::ParseResult::Exit)
			return CLI::ParseResult::Exit;

		if (!CommandLineParser::TryGetValue(inputOption, props.InputPath))
		{
			LogMessage("Error: Input file path was not specifed");
			return CLI::ParseResult::Exit;
		}

		if (!CommandLineParser::TryGetValue(outputOption, props.OutputPath))
			props.OutputPath = Path::ChangeExtension(props.InputPath, ".dat");

		return CLI::ParseResult::Continue;
	}

	CLI::ParseResult ConvertFont(int argc, char* argv[])
	{
		FontConversionProps props;

		if (ParseCommandLine(argc, argv, props) == CLI::ParseResult::Exit)
			return CLI::ParseResult::Exit;

		if (!File::Exists(props.InputPath))
		{
			LogMessage("Error: File \"%s\" does not exist", props.InputPath);
			return CLI::ParseResult::Exit;
		}

		Font font;
		if (!font.ReadBMFont(props.InputPath, props.TextureProps.Format))
		{
			return CLI::ParseResult::Exit;
		}

		TextureFlags& texFlags = font.TextureImage->GetFlags();
		texFlags.WrapS = props.TextureProps.WrapS;
		texFlags.WrapT = props.TextureProps.WrapT;
		texFlags.NearestFiltering = props.TextureProps.NearestFilter;

		TextureFileFlags texFileFlags{};
		texFileFlags.CompressedQOI = props.TextureProps.CompressQOI;
		texFileFlags.CompressedBZ2 = props.TextureProps.CompressBZ2;
		font.TextureImage->SetWriteFlags(texFileFlags);

		FileStream outputStream = File::CreateWrite(props.OutputPath);
		StreamWriter writer(outputStream);
		font.WriteBinary(writer);
			
		return CLI::ParseResult::Exit;
	}
}
