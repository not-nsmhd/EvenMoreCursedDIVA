#include "SpritePacker.h"
#include "IO/Path/Directory.h"
#include "IO/File.h"
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
					// TODO: Source shifting
					const Xml::Attribute* originX_Attrib = Xml::FindAttribute(optionsElement, "OriginX");
					const Xml::Attribute* originY_Attrib = Xml::FindAttribute(optionsElement, "OriginY");
					const Xml::Attribute* texAttrib = Xml::FindAttribute(optionsElement, "DesiredTextureIndex");

					vec2 newOrigin = spriteInfo.Origin;
					i32 newTexIndex = spriteInfo.DesiredTextureIndex;

					bool applyNewValues = false;

					if (originX_Attrib != nullptr) { applyNewValues = (originX_Attrib->QueryFloatValue(&newOrigin.x) == tinyxml2::XMLError::XML_SUCCESS); }
					if (originY_Attrib != nullptr) { applyNewValues = (originY_Attrib->QueryFloatValue(&newOrigin.y) == tinyxml2::XMLError::XML_SUCCESS); }
					if (texAttrib != nullptr) { applyNewValues = (texAttrib->QueryIntValue(&newTexIndex) == tinyxml2::XMLError::XML_SUCCESS); }

					if (applyNewValues)
					{
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

		if (ImageHelper::GetImageInfo(filePath, spriteInfo.Size, nullptr))
		{
			spriteInfo.Origin = vec2(
				static_cast<float>(spriteInfo.Size.x) / 2.0f,
				static_cast<float>(spriteInfo.Size.y) / 2.0f);

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

				i32 index = rectPacker.TryPack(it.Size);
				if (index != -1)
				{
					const Rectangle& packedRect = rectPacker.GetRectangle(index);
					it.WasPacked = true;
					it.PackedPosition.x = packedRect.X;
					it.PackedPosition.y = packedRect.Y;
				}

				continue;
			}

			ivec2 areaSize = rectPacker.GetRealAreaSize();
			ivec2 texSize_pow2 { MathExtensions::NearestPowerOf2(areaSize.x) + 1, MathExtensions::NearestPowerOf2(areaSize.y) + 1 };

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
			texInfo.DataSize = (texInfo.Size.x * texInfo.Size.y * 4ull);
			texInfo.Data = std::make_unique<u8[]>(texInfo.DataSize);

			for (auto& sprInfo : sprites)
			{
				if (sprInfo.DesiredTextureIndex != currentTexIndex || !sprInfo.WasPacked) { continue; }

				ImageHelper::ReadImageFile(sprInfo.ImagePath, size, channels, tempSpriteBuffer);

				size_t dstX = sprInfo.PackedPosition.x;
				size_t dstY = sprInfo.PackedPosition.y;

				for (size_t y = 0; y < sprInfo.Size.y; y++)
				{
					for (size_t x = 0; x < sprInfo.Size.x; x++)
					{
						Detail::GetPixel(dstX + x, dstY + y, texInfo.Size.x, texInfo.Data.get()) = 
							Detail::GetPixel(x, y, sprInfo.Size.x, tempSpriteBuffer.get());
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
