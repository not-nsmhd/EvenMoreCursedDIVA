#include "SpritePacker.h"
#include "IO/Path/Directory.h"
#include "IO/Path/File.h"
#include "IO/Xml.h"
#include "Misc/ImageHelper.h"
#include "Common/MathExt.h"
#include <algorithm>

namespace Starshine::GFX
{
	using namespace IO;
	using namespace Misc;

	namespace Detail
	{
		void ParseSpritePackOptions(const Xml::Document& doc, SpriteInfo& spriteInfo)
		{
			const Xml::Element* rootElement = Xml::GetRootElement(doc);
			if (rootElement != nullptr)
			{
				const Xml::Element* optionsElement = Xml::FindElement(rootElement, "SpritePackOptions");
				if (optionsElement != nullptr)
				{
					const Xml::Attribute* realSourceX_Attrib = Xml::FindAttribute(optionsElement, "RealSourceX");
					const Xml::Attribute* realSourceY_Attrib = Xml::FindAttribute(optionsElement, "RealSourceY");
					const Xml::Attribute* realWidth_Attrib = Xml::FindAttribute(optionsElement, "RealWidth");
					const Xml::Attribute* realHeight_Attrib = Xml::FindAttribute(optionsElement, "RealHeight");

					const Xml::Attribute* originX_Attrib = Xml::FindAttribute(optionsElement, "OriginX");
					const Xml::Attribute* originY_Attrib = Xml::FindAttribute(optionsElement, "OriginY");

					const Xml::Attribute* texAttrib = Xml::FindAttribute(optionsElement, "DesiredTextureIndex");

					vec2 newSource = spriteInfo.RealSource;
					vec2 newSize = spriteInfo.Size;
					vec2 newOrigin = spriteInfo.Origin;
					i32 newTexIndex = spriteInfo.DesiredTextureIndex;

					bool applyNewValues = false;

					if (realSourceX_Attrib != nullptr) { applyNewValues = (realSourceX_Attrib->QueryFloatValue(&newSource.x) == tinyxml2::XMLError::XML_SUCCESS); }
					if (realSourceY_Attrib != nullptr) { applyNewValues = (realSourceY_Attrib->QueryFloatValue(&newSource.y) == tinyxml2::XMLError::XML_SUCCESS); }
					if (realWidth_Attrib != nullptr)
					{ 
						applyNewValues = (realWidth_Attrib->QueryFloatValue(&newSize.x) == tinyxml2::XMLError::XML_SUCCESS);
						if (applyNewValues) { newOrigin.x = static_cast<float>(newSize.x) / 2.0f; }
					}

					if (realHeight_Attrib != nullptr)
					{
						applyNewValues = (realHeight_Attrib->QueryFloatValue(&newSize.y) == tinyxml2::XMLError::XML_SUCCESS);
						if (applyNewValues) { newOrigin.y = static_cast<float>(newSize.y) / 2.0f; }
					}

					if (originX_Attrib != nullptr) { applyNewValues = (originX_Attrib->QueryFloatValue(&newOrigin.x) == tinyxml2::XMLError::XML_SUCCESS); }
					if (originY_Attrib != nullptr) { applyNewValues = (originY_Attrib->QueryFloatValue(&newOrigin.y) == tinyxml2::XMLError::XML_SUCCESS); }

					if (texAttrib != nullptr) { applyNewValues = (texAttrib->QueryIntValue(&newTexIndex) == tinyxml2::XMLError::XML_SUCCESS); }

					if (applyNewValues)
					{
						spriteInfo.RealSource = newSource;
						spriteInfo.Size = newSize;
						spriteInfo.Origin = newOrigin;
						spriteInfo.DesiredTextureIndex = newTexIndex;
					}
				}
			}
		}

		u32& GetPixel(size_t x, size_t y, size_t width, void* rgbaPixels)
		{
			return reinterpret_cast<u32*>(rgbaPixels)[(y * width) + x];
		}
	}

	void SpritePacker::Initialize()
	{
		rectPacker.Initialize();
		texturesToReserve = 1;
	}

	void SpritePacker::Clear()
	{
		rectPacker.Clear();
		sprites.clear();
		textures.clear();
	}

	bool SpritePacker::AddImage(std::string_view filePath)
	{
		std::string_view fileName = Path::GetFileName(filePath, false);
		if (Path::GetExtension(filePath) != ".png")
		{
			return false;
		}

		SpriteInfo spriteInfo{};

		if (ImageHelper::GetImageInfo(filePath, spriteInfo.ImageSize, nullptr))
		{
			spriteInfo.Origin = vec2(
				static_cast<float>(spriteInfo.ImageSize.x) / 2.0f,
				static_cast<float>(spriteInfo.ImageSize.y) / 2.0f);

			spriteInfo.Size = spriteInfo.ImageSize;

			spriteInfo.ImagePath = filePath;
			spriteInfo.Name = fileName;

			spriteInfo.OriginalIndex = static_cast<i32>(sprites.size());

			// Check if the corresponding XML file is also present
			std::string xmlFilePath = Path::ChangeExtension(filePath, ".xml");
			if (File::Exists(xmlFilePath))
			{
				Xml::Document doc;
				if (Xml::ParseFromFile(doc, xmlFilePath))
				{
					Detail::ParseSpritePackOptions(doc, spriteInfo);
					if ((static_cast<size_t>(spriteInfo.DesiredTextureIndex) + 1ull) > texturesToReserve)
					{ 
						texturesToReserve = static_cast<size_t>(spriteInfo.DesiredTextureIndex) + 1ull;
					}
				}

				doc.Clear();
			}

			sprites.push_back(spriteInfo);
			return true;
		}

		return false;
	}

	void SpritePacker::Pack()
	{
		SortSpritesByArea();
		SortSpritesByTextureIndex();

		textures.reserve(texturesToReserve);

		for (size_t i = 0; i < texturesToReserve; i++)
		{
			for (auto& it : sprites)
			{
				if (it.DesiredTextureIndex != i) { continue; }

				i32 index = rectPacker.TryPack(it.ImageSize);
				if (index != -1)
				{
					const Rectangle& packedRect = rectPacker.GetRectangle(index);
					it.WasPacked = true;
					it.SheetPosition.x = packedRect.X;
					it.SheetPosition.y = packedRect.Y;
					it.PackedPosition.x = packedRect.X + it.RealSource.x;
					it.PackedPosition.y = packedRect.Y + it.RealSource.y;
				}

				continue;
			}

			ivec2 areaSize = rectPacker.GetRealAreaSize();
			ivec2 texSize_pow2 { MathExtensions::NearestPowerOf2(areaSize.x), MathExtensions::NearestPowerOf2(areaSize.y) };

			textures.emplace_back(SheetTextureInfo
				{
					texSize_pow2,
					rectPacker.GetRealAreaSize(),
					rectPacker.GetRectangleCount(),
					0 
				});

			// Reinitialize rectangle packer for the next texture
			rectPacker.Clear();
			rectPacker.Initialize();
		}

		GenerateSheetTextures();
		RestoreOriginalSpriteOrder();
	}

	void SpritePacker::SortSpritesByArea()
	{
		// NOTE: From biggest to smallest
		std::sort(sprites.begin(), sprites.end(), [](SpriteInfo& sprA, SpriteInfo& sprB)
			{
				return (sprA.Size.x * sprA.Size.y) > (sprB.Size.x * sprB.Size.y);
			});
	}

	void SpritePacker::SortSpritesByTextureIndex()
	{
		std::sort(sprites.begin(), sprites.end(), [](SpriteInfo& sprA, SpriteInfo& sprB)
			{
				return sprA.DesiredTextureIndex < sprB.DesiredTextureIndex;
			});
	}

	void SpritePacker::RestoreOriginalSpriteOrder()
	{
		std::sort(sprites.begin(), sprites.end(), [](SpriteInfo& sprA, SpriteInfo& sprB)
			{
				return sprA.OriginalIndex < sprB.OriginalIndex;
			});
	}

	void SpritePacker::GenerateSheetTextures()
	{
		i32 currentTexIndex = 0;
		std::unique_ptr<u8[]> tempSpriteBuffer;

		ivec2 size{};
		i32 channels{};

		for (auto& texInfo : textures)
		{
			constexpr size_t rgbaPixelSize = 4ull;

			texInfo.DataSize = (static_cast<size_t>(texInfo.Size.x) * static_cast<size_t>(texInfo.Size.y) * rgbaPixelSize);
			texInfo.Data = std::make_unique<u8[]>(texInfo.DataSize);

			for (auto& sprInfo : sprites)
			{
				if (sprInfo.DesiredTextureIndex != currentTexIndex || !sprInfo.WasPacked) { continue; }

				ImageHelper::ReadImageFile(sprInfo.ImagePath, size, channels, tempSpriteBuffer);

				size_t dstX = sprInfo.SheetPosition.x;
				size_t dstY = sprInfo.SheetPosition.y;

				for (size_t y = 0; y < sprInfo.ImageSize.y; y++)
				{
					for (size_t x = 0; x < sprInfo.ImageSize.x; x++)
					{
						Detail::GetPixel(dstX + x, dstY + y, texInfo.Size.x, texInfo.Data.get()) = 
							Detail::GetPixel(x, y, sprInfo.ImageSize.x, tempSpriteBuffer.get());
					}
				}
			}

			currentTexIndex++;
		}
	}

	const SpriteInfo* SpritePacker::GetSpriteInfo(i32 index) const
	{
		if (static_cast<size_t>(index) < sprites.size())
		{
			return &sprites[index];
		}

		return nullptr;
	}

	size_t SpritePacker::GetSpriteCount() const
	{
		return sprites.size();
	}

	const SheetTextureInfo* SpritePacker::GetTextureInfo(i32 index) const
	{
		if (static_cast<size_t>(index) < textures.size())
		{
			return &textures[index];
		}

		return nullptr;
	}

	size_t SpritePacker::GetTextureCount() const
	{
		return textures.size();
	}
}
