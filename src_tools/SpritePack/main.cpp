#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <stb_image.h>
#include <stb_image_write.h>
#include <cstring>
#include <algorithm>
#include <fmt/format.h>
#include <qoi.h>
#include "int_types.h"
#include "types.h"
#include "rect_pack.h"
#include "io/binary_writer.h"

using namespace std::filesystem;

void WriteText(std::vector<Sprite>& spritesToPack, std::vector<path>& spriteImagePaths, i32 sheetWidth, i32 sheetHeight, std::string name)
{
	size_t sheetImageSize = static_cast<size_t>(sheetWidth * sheetHeight * 4);
	u8* sheetImageData = new u8[sheetImageSize];
	memset(sheetImageData, 0, sheetImageSize);

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
	for (std::vector<Sprite>::const_iterator it = spritesToPack.cbegin(); it != spritesToPack.cend(); it++)
	{
		if (!it->WasPacked)
		{
			continue;
		}

		spriteName = spriteImagePaths[it->Index].filename().replace_extension("").string();
		outputMapFile << spriteName;
		outputMapFile << " = ";
		outputMapFile << fmt::format("{0} {1} {2} {3} {4} {5:.3g} {6:.3g}\n", 
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

	sheetWidth = nextPowerOf2(sheetWidth);
	sheetHeight = nextPowerOf2(sheetHeight);

	WriteText(spritesToPack, spriteImagePaths, sheetWidth, sheetHeight, argv[2]);

	/*u8* sheetImageData = new u8[sheetWidth * sheetHeight * 4];

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

	qoi_desc qoiSheetDesc = {};
	qoiSheetDesc.width = sheetWidth;
	qoiSheetDesc.height = sheetHeight;
	qoiSheetDesc.channels = 4;

	int qoiSheetSize = 0;
	void* qoiSheet = qoi_encode(sheetImageData, &qoiSheetDesc, &qoiSheetSize);

	path sheetPath = std::filesystem::absolute(argv[2]);
	sheetPath.replace_extension(".dat");
	std::fstream testBinaryStream(sheetPath, std::ios::out | std::ios::binary);

	if (testBinaryStream.fail())
	{
		std::cerr << sheetPath << " failed to create\n";
		return -1;
	}

	BinaryWriter binaryWriter(&testBinaryStream, 4);

	binaryWriter.WriteU8('S');
	binaryWriter.WriteU8('P');
	binaryWriter.WriteU8('R');
	binaryWriter.WriteU8(0);

	binaryWriter.WriteU16(1);
	binaryWriter.WriteU16(1);

	binaryWriter.WriteU32(spritesToPack.size());
	binaryWriter.WriteU32(1);

	for (std::vector<Sprite>::const_iterator it = spritesToPack.cbegin(); it != spritesToPack.cend(); it++)
	{
		binaryWriter.WriteU16(it->Index);

		binaryWriter.WriteU16(it->TexIndex);

		binaryWriter.WriteU16(it->X);
		binaryWriter.WriteU16(it->Y);
		binaryWriter.WriteU16(it->Width);
		binaryWriter.WriteU16(it->Height);

		binaryWriter.WriteFloat(it->OriginX);
		binaryWriter.WriteFloat(it->OriginY);
	}

	binaryWriter.PadUntilAligned();

	std::string spriteName = "";
	for (size_t i = 0; i < spriteImagePaths.size(); i++)
	{
		spriteName = spriteImagePaths[i].filename().replace_extension("").string();
		binaryWriter.WriteLengthPrefixedString(spriteName);
		binaryWriter.PadUntilAligned();
	}

	binaryWriter.WriteU16(sheetWidth);
	binaryWriter.WriteU16(sheetHeight);
	binaryWriter.Write(sheetImageData, sheetWidth * sheetHeight * 4);

	testBinaryStream.close();
	delete[] sheetImageData;
	free(qoiSheet);*/

	return 0;
}
