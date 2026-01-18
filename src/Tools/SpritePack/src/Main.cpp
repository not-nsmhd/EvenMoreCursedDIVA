#include <Common/Types.h>
#include <IO/Path/File.h>
#include <IO/FileStream.h>
#include <IO/StreamWriter.h>
#include <GFX/SpritePacker.h>

#define QOI_IMPLEMENTATION
#define QOI_NO_STDIO
#include "QOI/qoi.h"

#include <zlib.h>

constexpr char CurrentRevision = 0;
constexpr std::array<char, 4> FileSignature { 'S', 'P', 'R', CurrentRevision };

enum SpriteTextureFlags : u16
{
	STXFlags_None = 0,

	STXFlags_NearestFilter = (1 << 0),
	STXFlags_ClampS = (1 << 1),
	STXFlags_ClampT = (1 << 2),

	STXFlags_CompressedQOI = (1 << 8),
	STXFlags_CompressedZLIB = (1 << 9), // TODO: Implement
};

int main(int argc, char* argv[])
{
	using namespace Starshine;
	using namespace Starshine::GFX;
	using namespace Starshine::IO;

	SpritePacker sprPacker;
	sprPacker.Initialize();
	sprPacker.AddFromDirectory("spr_devtest2");
	sprPacker.Pack();

	FileStream outputStream = File::CreateWrite("spr_devtest2.dat");
	StreamWriter writer = StreamWriter(outputStream);

	writer.WriteBuffer(FileSignature.data(), FileSignature.size());

	const size_t spriteCount = sprPacker.GetSpriteCount();
	const size_t textureCount = sprPacker.GetTextureCount();

	writer.WriteSize(spriteCount);
	writer.WriteFunctionPointer([&](StreamWriter& writer)
		{
			for (size_t i = 0; i < spriteCount; i++)
			{
				const SpriteInfo* sprite = sprPacker.GetSpriteInfo(i);

				writer.WriteStringPointer(sprite->Name, 16);
				writer.WriteI32(sprite->DesiredTextureIndex);

				writer.WriteU16(sprite->PackedPosition.x);
				writer.WriteU16(sprite->PackedPosition.y);
				writer.WriteU16(sprite->Size.x);
				writer.WriteU16(sprite->Size.y);

				writer.WriteF32(sprite->Origin.x);
				writer.WriteF32(sprite->Origin.y);
			}

			writer.FlushStringArray();
		});

	writer.WriteSize(textureCount);
	writer.WriteFunctionPointer([&](StreamWriter& writer)
		{
			for (size_t i = 0; i < textureCount; i++)
			{
				const SheetTextureInfo* tex = sprPacker.GetTextureInfo(i);

				writer.WriteU16(tex->Size.x);
				writer.WriteU16(tex->Size.y);

				writer.WriteU16(0); // Base Format (RGBA8)
				writer.WriteU16(STXFlags_CompressedQOI);

				qoi_desc qoiDesc {};
				qoiDesc.width = tex->Size.x;
				qoiDesc.height = tex->Size.y;
				qoiDesc.channels = 4;

				int qoiDataLength = 0;
				void* qoiData = qoi_encode(tex->Data.get(), &qoiDesc, &qoiDataLength);

				writer.WriteSize(tex->DataSize);
				writer.WriteSize(qoiDataLength);
				writer.WriteBuffer(qoiData, qoiDataLength);

				QOI_FREE(qoiData);
			}
		});

	writer.WriteU32(0);
	writer.WriteU32(0);
	writer.WriteU32(0);

	writer.FlushFunctionArray();

	outputStream.Close();
	sprPacker.Clear();

	return 0;
}
