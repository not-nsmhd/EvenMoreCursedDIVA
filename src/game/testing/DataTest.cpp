#include "DataTest.h"
#include "gfx/Renderer.h"
#include "util/logging.h"

#include "IO/FileStream.h"
#include "IO/StreamWriter.h"
#include "IO/StreamReader.h"
#include <array>
#include <string>
#include <vector>

namespace Starshine::Testing
{
	using namespace GFX;
	using namespace IO;

	struct DataTest::Impl
	{
		static constexpr std::array<char, 4> FileSignature{ 'T', 'E', 'S', 'T' };

		struct DataStructure
		{
			std::string Name;

			u16 X{};
			u16 Y{};
			u16 Width{};
			u16 Height{};

			f32 OriginX{};
			f32 OriginY{};
		};

		std::vector<DataStructure> Data;

		Impl() {}

		void CreateTestData(size_t amount)
		{
			Data.reserve(amount);

			u16 xOffset = 0;
			u16 yOffset = 0;

			char nameBuffer[32]{};
			for (size_t i = 0; i < amount; i++)
			{
				SDL_snprintf(nameBuffer, sizeof(nameBuffer) - 1, "Element_%02llx", i);

				Data.push_back({ nameBuffer, xOffset, yOffset, 256, 256, 128.0f, 128.0f});
				xOffset += 256 + 1;
				if (xOffset >= 1024)
				{
					xOffset = 0;
					yOffset += 256 + 1;
				}
			}
		}

		void WriteTestData()
		{
			FileStream testFile;
			testFile.CreateWrite("DataTest.dat");

			StreamWriter writer = StreamWriter(testFile);
			writer.WriteBuffer(FileSignature.data(), FileSignature.size());

			writer.WriteSize(Data.size());
			writer.WriteAlignedPadding(8, true);

			for (auto& item : Data)
			{
				writer.WriteStringPointer(item.Name, 8);

				writer.WriteU16(item.X);
				writer.WriteU16(item.Y);
				writer.WriteU16(item.Width);
				writer.WriteU16(item.Height);

				writer.WriteF32(item.OriginX);
				writer.WriteF32(item.OriginY);
			}

			writer.FlushStringArray();
			testFile.Close();
		}

		bool ReadTestData()
		{
			FileStream testFile;
			testFile.OpenRead("DataTest.dat");

			if (!testFile.IsOpen())
			{
				return false;
			}

			StreamReader reader = StreamReader(testFile);

			std::array<u8, 4> signature;
			reader.ReadBuffer(signature.data(), FileSignature.size());

			if (memcmp(signature.data(), FileSignature.data(), FileSignature.size()) != 0)
			{
				testFile.Close();
				return false;
			}

			size_t elementCount = reader.ReadSize();
			Data.reserve(elementCount);
			reader.SkipUntilAligned(8, true);

			for (size_t i = 0; i < elementCount; i++)
			{
				std::string name = reader.ReadStringPointer();

				u16 x = reader.ReadU16();
				u16 y = reader.ReadU16();
				u16 width = reader.ReadU16();
				u16 height = reader.ReadU16();

				f32 originX = reader.ReadF32();
				f32 originY = reader.ReadF32();

				Data.push_back({ name, x, y, width, height, originX, originY });
			}

			testFile.Close();
			return true;
		}
	};

	DataTest::DataTest()
	{
	}

	bool DataTest::Initialize()
	{
		impl = new DataTest::Impl();
		return true;
	}

	bool DataTest::LoadContent()
	{
		//impl->CreateTestData(16);
		//impl->WriteTestData();
		
		if (impl->ReadTestData() != true)
		{
			return false;
		}

		return true;
	}

	void DataTest::UnloadContent()
	{
	}

	void DataTest::Destroy()
	{
		delete impl;
	}

	void DataTest::Update(f64 deltaTime_milliseconds)
	{
	}

	void DataTest::Draw(f64 deltaTime_milliseconds)
	{
		Renderer::GetInstance()->Clear(ClearFlags_Color, { 24, 24, 24, 255 }, 1.0f, 0);
		Renderer::GetInstance()->SwapBuffers();
	}

	std::string_view DataTest::GetStateName() const
	{
		return "[Dev] Data Test";
	}
}
