#include <SDL2/SDL_main.h>
#include <Common/Logging/Logging.h>
#include "Converters/TextureConverter.h"
#include "Converters/FontConverter.h"
#include "Converters/Bzip2Compression.h"
#include "BuildInfo.h"

using namespace Starshine;
using namespace DataBuild;

void PrintInfo()
{
	LogMessage("Starshine Data Build");
	LogMessage("Build Info (StarshineLib):");

#ifdef _DEBUG
	LogMessage("\tConfiguration: Debug");
#else
	LogMessage("\tConfiguration: Release");
#endif

	LogMessage("\tBuild Date: %s", BuildInfo::BuildDateString);
	LogMessage("\tGit Branch and Commit: %s - %s\n", BuildInfo::GitBranchName, BuildInfo::GitCommitHashString);
}

void PrintModes(const char* executableName)
{
	LogMessage("Usage: %s [mode] [mode flags]", executableName);
	LogMessage("Modes:");

	LogMessage("\ttexture - Convert textures");
	LogMessage("\tfont - Convert bitmap fonts");
	LogMessage("\tcompress - Compress file using bzip2");
	LogMessage("\tdecompress - Decompress bzip2 file");

	LogMessage("\nType \"[mode] --help\" to get flags for a specific mode");
}

int SDL_main(int argc, char* argv[])
{
	if (argc == 1)
	{
		PrintInfo();
		PrintModes(argv[0]);
		return 0;
	}

	const char* mode = argv[1];

	i32 modeArgc = argc - 2;
	char** modeArgv = &argv[2];

	if (SDL_strcmp(mode, "texture") == 0)
		ConvertTexture(modeArgc, modeArgv);
	else if (SDL_strcmp(mode, "font") == 0)
		ConvertFont(modeArgc, modeArgv);
	else if (SDL_strcmp(mode, "compress") == 0)
		CompressFile(modeArgc, modeArgv);
	else if (SDL_strcmp(mode, "decompress") == 0)
		DecompressFile(modeArgc, modeArgv);
	else if (SDL_strcmp(mode, "help") == 0)
		PrintModes(argv[0]);
	else if (SDL_strcmp(mode, "version") == 0)
		PrintInfo();
	else
	{
		LogMessage("Unknown mode: \"%s\"", mode);
		PrintModes(argv[0]);
	}

	return 0;
}
