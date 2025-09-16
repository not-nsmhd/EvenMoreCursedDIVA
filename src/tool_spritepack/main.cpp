#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <stb_image.h>
#include <stb_image_write.h>
#include <cstring>
#include <algorithm>
#include "int_types.h"
#include "types.h"
#include "rect_pack.h"
#include "io/binary_writer.h"

using namespace std::filesystem;

void WriteText(std::vector<Sprite>& spritesToPack, std::vector<path>& spriteImagePaths, i32 sheetWidth, i32 sheetHeight, std::string name)
{
	size_t sheetImageSize = static_cast<size_t>(sheetWidth) * static_cast<size_t>(sheetHeight) * 4;
	u8* sheetImageData = new u8[sheetImageSize];
	
	for (size_t i = 0; i < sheetImageSize; i += 4)
	{
		sheetImageData[i + 0] = 64;
		sheetImageData[i + 1] = 64;
		sheetImageData[i + 2] = 64;
		sheetImageData[i + 3] = 255;
	}

	int dummy = 0;
	for (std::vector<Sprite>::iterator it = spritesToPack.begin(); it != spritesToPack.end(); it++)
	{
		std::string spriteImagePath = spriteImagePaths[it->Index].string();
		u8* spriteImage = stbi_load(spriteImagePath.c_str(), &dummy, &dummy, &dummy, 4);

		for (size_t y = 0; y < it->Height; y++)
		{
			for (size_t x = 0; x < it->Width; x++)
			{
				if (spriteImage[(y * it->Width + x) * 4 + 3] == 0)
				{
					sheetImageData[((y + it->Y) * sheetWidth + (x + it->X)) * 4 + 0] = 0;
					sheetImageData[((y + it->Y) * sheetWidth + (x + it->X)) * 4 + 1] = 0;
					sheetImageData[((y + it->Y) * sheetWidth + (x + it->X)) * 4 + 2] = 0;
					sheetImageData[((y + it->Y) * sheetWidth + (x + it->X)) * 4 + 3] = 0;
				}
				else
				{
					sheetImageData[((y + it->Y) * sheetWidth + (x + it->X)) * 4 + 0] = spriteImage[(y * it->Width + x) * 4 + 0];
					sheetImageData[((y + it->Y) * sheetWidth + (x + it->X)) * 4 + 1] = spriteImage[(y * it->Width + x) * 4 + 1];
					sheetImageData[((y + it->Y) * sheetWidth + (x + it->X)) * 4 + 2] = spriteImage[(y * it->Width + x) * 4 + 2];
					sheetImageData[((y + it->Y) * sheetWidth + (x + it->X)) * 4 + 3] = spriteImage[(y * it->Width + x) * 4 + 3];
				}
			}
		}

		stbi_image_free(spriteImage);
	}

	path outputName(name);
	if (!std::filesystem::exists(outputName))
	{
		std::filesystem::create_directory(outputName);
	}

	path sheetPath = outputName;
	sheetPath /= "sheet_0.png";

	stbi_write_png(sheetPath.string().c_str(), sheetWidth, sheetHeight, 4, sheetImageData, sheetWidth * 4);
	delete[] sheetImageData;

	path mapPath = outputName;
	mapPath /= "map.txt";

	std::fstream outputMapFile(mapPath, std::ios::out | std::ios::binary);

	std::string spriteName;
	char spritePropertiesString[512] {};
	for (std::vector<Sprite>::const_iterator it = spritesToPack.cbegin(); it != spritesToPack.cend(); it++)
	{
		if (!it->WasPacked)
		{
			continue;
		}

		spriteName = spriteImagePaths[it->Index].filename().replace_extension("").string();
		outputMapFile << spriteName;
		outputMapFile << " = ";
		//outputMapFile << fmt::format("{0} {1} {2} {3} {4} {5:.3g} {6:.3g}\n", 
			//it->TexIndex, it->X, it->Y, it->Width, it->Height, it->OriginX, it->OriginY);

		snprintf(spritePropertiesString, sizeof(spritePropertiesString) - 1, 
			"%d %d %d %d %d %.3f %.3f", 
			it->TexIndex, it->X, it->Y, it->Width, it->Height, it->OriginX, it->OriginY);
	}
	
	outputMapFile.close();
}

int nextPowerOf2(int value)
{
	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value++;
	return value;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		return -1;
	}

	path srcDir = std::filesystem::absolute(argv[1]);
	if (!is_directory(srcDir))
	{
		std::cerr << srcDir << " is not a directory\n";
		return -1;
	}

	std::vector<Sprite> spritesToPack;
	std::vector<path> spriteImagePaths;

	u32 spriteCount = 0;
	for (auto const& dirEntry : directory_iterator(srcDir))
	{
		if (!dirEntry.is_regular_file())
		{
			continue;
		}

		std::string spritePath = dirEntry.path().string();

		Sprite newSprite = {};
		if (!stbi_info(spritePath.c_str(), &newSprite.Width, &newSprite.Height, NULL))
		{
			continue;
		}

		//newSprite.Name = dirEntry.path().filename().replace_extension().string();
		newSprite.Index = spriteCount++;
		newSprite.OriginX = static_cast<float>(newSprite.Width) / 2.0f;
		newSprite.OriginY = static_cast<float>(newSprite.Height) / 2.0f;

		spritesToPack.push_back(newSprite);
		spriteImagePaths.push_back(spritePath);
	}

	RectanglePacker rectPack = RectanglePacker(2048, 2048, 2, 2);
	std::sort(spritesToPack.begin(), spritesToPack.end(), CompareByArea_Inverted);
	
	for (std::vector<Sprite>::iterator it = spritesToPack.begin(); it != spritesToPack.end(); it++)
	{
		i32 placementX = 0;
		i32 placementY = 0;
		if (rectPack.TryPack(it->Width, it->Height, &placementX, &placementY))
		{
			it->X = placementX;
			it->Y = placementY;
			it->WasPacked = true;
		}
	}
	
	std::sort(spritesToPack.begin(), spritesToPack.end(), CompareByIndex);

	int sheetWidth = 0;
	int sheetHeight = 0;
	rectPack.GetActualPackArea(&sheetWidth, &sheetHeight);

	//sheetWidth = nextPowerOf2(sheetWidth);
	//sheetHeight = nextPowerOf2(sheetHeight);

	WriteText(spritesToPack, spriteImagePaths, sheetWidth, sheetHeight, argv[2]);

	return 0;
}
