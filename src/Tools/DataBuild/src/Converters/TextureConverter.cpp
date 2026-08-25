#include "TextureConverter.h"
#include <Common/Logging/Logging.h>
#include <IO/Path/Path.h>
#include <IO/Path/File.h>

using namespace Starshine;
using namespace Starshine::IO;
using namespace Starshine::Graphics;
using namespace Starshine::Misc;
using namespace Starshine::Misc::ImageHelper;
using Starshine::CLI::CommandLineParser;
using Starshine::CLI::CommandLineOptionDesc;

namespace DataBuild
{
	TextureFormat ParseFormatString(std::string_view formatString)
	{
		i32 formatIndex = 0;
		for (const auto& name : TextureFormatNames)
		{
			if (name == formatString)
				return static_cast<TextureFormat>(formatIndex);

			formatIndex++;
		}

		LogMessage("Warning: Unknown texture format \"%s\". Texture will be encoded in RGBA8 format", formatString.data());
		return TextureFormat::RGBA8;
	}

	ImageFormat ParseExtractionFormatString(std::string_view formatString)
	{
		i32 formatIndex = 0;
		for (const auto& name : ImageFormatNames)
		{
			if (name == formatString)
				return static_cast<ImageFormat>(formatIndex);

			formatIndex++;
		}

		LogMessage("Warning: Unknown format \"%s\". Image will be saved in PNG format", formatString.data());
		return ImageFormat::PNG;
	}

	CLI::ParseResult ParseTextureConversionCommandLine(int argc, char* argv[], TextureConversionProps& props)
	{
		constexpr CommandLineOptionDesc formatOption("--format", "-f", "Texture format");

		constexpr CommandLineOptionDesc wrapSFlag("--wrap-s", nullptr, "Enable texture wrapping on X coordinate", true);
		constexpr CommandLineOptionDesc wrapTFlag("--wrap-t", nullptr, "Enable texture wrapping on Y coordinate", true);
		constexpr CommandLineOptionDesc nearestFlag("--nearest", nullptr, "Set nearest filter", true);

		constexpr CommandLineOptionDesc qoiFlag("--compress-qoi", nullptr, "Enable QOI compression (only for RGBA8 textures)", true);
		constexpr CommandLineOptionDesc bz2Flag("--compress-bz2", nullptr, "Enable bzip2 compression", true);

		CommandLineParser::AddOption(formatOption);

		CommandLineParser::AddOption(wrapSFlag);
		CommandLineParser::AddOption(wrapTFlag);
		CommandLineParser::AddOption(nearestFlag);

		CommandLineParser::AddOption(qoiFlag);
		CommandLineParser::AddOption(bz2Flag);

		if (CommandLineParser::Parse(argc, argv) == CLI::ParseResult::Exit)
			return CLI::ParseResult::Exit;

		props.WrapS = CommandLineParser::IsOptionPresent(wrapSFlag);
		props.WrapT = CommandLineParser::IsOptionPresent(wrapTFlag);
		props.NearestFilter = CommandLineParser::IsOptionPresent(nearestFlag);
		props.CompressQOI = CommandLineParser::IsOptionPresent(qoiFlag);
		props.CompressBZ2 = CommandLineParser::IsOptionPresent(bz2Flag);

		std::string formatString;
		if (CommandLineParser::TryGetValue(formatOption, formatString))
		{
			if (!props.Extract)
				props.Format = ParseFormatString(formatString);
			else
				props.ExtractionFormat = ParseExtractionFormatString(formatString);
		}

		return CLI::ParseResult::Continue;
	}

	CLI::ParseResult ParseFullCommandLine(int argc, char* argv[], TextureConversionProps& props)
	{
		constexpr CommandLineOptionDesc inputOption("--input", "-i", "Input file path");
		constexpr CommandLineOptionDesc outputOption("--output", "-o", "Output file path");
		constexpr CommandLineOptionDesc extractFlag("--extract", "-x", "Convert texture file back to source format", true);

		CommandLineParser::AddOption(inputOption);
		CommandLineParser::AddOption(outputOption);
		CommandLineParser::AddOption(extractFlag);

		if (ParseTextureConversionCommandLine(argc, argv, props) == CLI::ParseResult::Exit)
			return CLI::ParseResult::Exit;

		props.Extract = CommandLineParser::IsOptionPresent(extractFlag);

		if (!CommandLineParser::TryGetValue(inputOption, props.InputPath))
		{
			LogMessage("Error: Input file path was not specifed");
			return CLI::ParseResult::Exit;
		}

		if (!CommandLineParser::TryGetValue(outputOption, props.OutputPath))
			props.OutputPath = Path::ChangeExtension(props.InputPath, props.Extract ? ".png" : ".stx");

		return CLI::ParseResult::Continue;
	}

	CLI::ParseResult ConvertTexture(int argc, char* argv[])
	{
		TextureConversionProps props{};

		if (ParseFullCommandLine(argc, argv, props) == CLI::ParseResult::Exit)
			return CLI::ParseResult::Exit;

		if (!File::Exists(props.InputPath))
		{
			LogMessage("Error: File \"%s\" does not exist", props.InputPath);
			return CLI::ParseResult::Exit;
		}

		std::unique_ptr<Texture> tex;
		if (!props.Extract)
		{
			if (!Misc::ImageHelper::ReadImageFile(props.InputPath, tex, props.Format))
			{
				LogMessage("Error: Failed to open image file \"%s\"", props.InputPath);
				return CLI::ParseResult::Exit;
			}

			TextureFlags& texFlags = tex->GetFlags();
			TextureFileFlags texFileFlags{};

			texFlags.WrapS = props.WrapS;
			texFlags.WrapT = props.WrapT;
			texFlags.NearestFiltering = props.NearestFilter;

			texFileFlags.CompressedQOI = props.CompressQOI && (props.Format == TextureFormat::RGBA8);
			texFileFlags.CompressedBZ2 = props.CompressBZ2;

			FileStream outputStream = File::CreateWrite(props.OutputPath);
			StreamWriter writer(outputStream);
			tex->SetWriteFlags(texFileFlags);
			tex->WriteBinary(writer);
		}
		else
		{
			tex = std::make_unique<Texture>();

			FileStream inputStream = File::OpenRead(props.InputPath);
			StreamReader reader(inputStream);
			if (!tex->ReadBinary(reader))
			{
				LogMessage("Error: Failed to open texture file \"%s\"", props.InputPath);
				return CLI::ParseResult::Exit;
			}

			inputStream.Close();

			ImageHelper::WriteImageFile(props.OutputPath, props.ExtractionFormat, tex->GetData(), tex->GetSize(), tex->GetFormat());
		}

		return CLI::ParseResult::Exit;
	}
}
