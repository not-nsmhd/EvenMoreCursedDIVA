#include "FontTest.h"
#include "Definitions.h"
#include <IO/StreamReader.h>
#include <IO/Path/File.h>
#include <Rendering/Texture.h>
#include <Rendering/Render2D/SpriteRenderer.h>
#include <utf8.h>
#include <GameContext.h>

namespace Sandbox
{
	using namespace Starshine;
	using namespace Starshine::Rendering::Render2D;

	namespace BinFontFormat
	{
		static constexpr char CurrentRevision{ 0 };
		static constexpr std::array<char, 4> FontSignature{ 'S', 'F', 'T', CurrentRevision };

		struct GlyphRangeMapping
		{
			i32 CharCode{};
			i32 FirstGlyph{};
			size_t GlyphCount{};
		};

		struct GlyphKerning
		{
			i32 SecondCharCode{};
			i32 Amount{};
		};

		struct GlyphKerningList
		{
			i32 CharCode{};
			std::vector<GlyphKerning> KerningList;
		};

		struct FontGlyph
		{
			u16 X{};
			u16 Y{};
			u16 Width{};
			u16 Height{};

			i16 XOffset{};
			i16 YOffset{};

			i16 XAdvance{};
			GlyphKerningList* KerningList{};
		};

		class NewFont : public NonCopyable
		{
		public:
			NewFont() {};
			~NewFont() = default;

		private:
			struct FontDescription
			{
				i32 GlyphSize{};
				i32 LineHeight{};
				i32 Baseline{};
			} description = {};

			std::vector<FontGlyph> glyphs{};
			std::vector<GlyphKerningList> kernings{};

			std::vector<GlyphRangeMapping> glyphCharMappings{};

			Rendering::Texture* texture{};

			FontGlyph* getGlyph(i32 charCode)
			{
				if (glyphCharMappings.size() == 0) { return nullptr; }
				FontGlyph* result{};

				for (const auto& mapping : glyphCharMappings)
				{
					if (charCode >= mapping.CharCode && charCode < (mapping.CharCode + mapping.GlyphCount))
					{
						return &glyphs[mapping.FirstGlyph + (charCode - mapping.CharCode)];
					}
				}

				return &glyphs[0];
			}

			void resolveKerningReferences()
			{
				if (kernings.size() > 0)
				{
					for (auto& kernList : kernings)
					{
						FontGlyph* glyph = getGlyph(kernList.CharCode);
						glyph->KerningList = &kernList;
					}
				}
			}

		public:
			bool Read(IO::StreamReader& reader)
			{
				reader.SetPointerSize(IO::PointerSize::Size32Bit);

				char signature[4]{};
				reader.ReadBuffer(signature, sizeof(signature));

				if (SDL_memcmp(signature, FontSignature.data(), FontSignature.size()) != 0) { return false; }

				description.GlyphSize = reader.ReadI32();
				description.LineHeight = reader.ReadI32();
				description.Baseline = reader.ReadI32();

				const size_t glyphCount = reader.ReadSize();
				const size_t glyphArrayOffset = reader.ReadPointer();

				glyphs.reserve(glyphCount);
				reader.ReadAt(glyphArrayOffset, [&](IO::StreamReader& reader)
					{
						for (size_t i = 0; i < glyphCount; i++)
						{
							FontGlyph glyph{};

							glyph.X = reader.ReadU16();
							glyph.Y = reader.ReadU16();
							glyph.Width = reader.ReadU16();
							glyph.Height = reader.ReadU16();
							
							glyph.XOffset = reader.ReadI16();
							glyph.YOffset = reader.ReadI16();
							glyph.XAdvance = reader.ReadI16();

							glyphs.push_back(glyph);
							reader.SkipUntilAligned(16);
						}
					});

				const size_t charMappingCount = reader.ReadSize();
				const size_t charMappingOffset = reader.ReadPointer();

				glyphCharMappings.reserve(charMappingCount);
				reader.ReadAt(charMappingOffset, [&](IO::StreamReader& reader)
					{
						for (size_t i = 0; i < charMappingCount; i++)
						{
							GlyphRangeMapping mapping{};

							mapping.CharCode = reader.ReadI32();
							mapping.FirstGlyph = reader.ReadI32();
							mapping.GlyphCount = reader.ReadSize();

							glyphCharMappings.push_back(mapping);
							reader.SkipUntilAligned(16);
						}
					});

				const size_t kerningCount = reader.ReadSize();
				const size_t kerningOffset = reader.ReadPointer();

				if (kerningCount > 0 && kerningOffset > 0)
				{
					kernings.reserve(kerningCount);
					reader.ReadAt(kerningOffset, [&](IO::StreamReader& reader)
						{
							for (size_t i = 0; i < kerningCount; i++)
							{
								GlyphKerningList kernList{};

								kernList.CharCode = reader.ReadI32();
								size_t kernListSize = reader.ReadSize();

								kernList.KerningList.reserve(kernListSize);
								for (size_t j = 0; j < kernListSize; j++)
								{
									GlyphKerning kern{};

									kern.SecondCharCode = reader.ReadI32();
									kern.Amount = reader.ReadI32();

									kernList.KerningList.push_back(kern);
								}

								kernings.push_back(kernList);
								reader.SkipUntilAligned(16);
							}
						});

					resolveKerningReferences();
				}

				return true;
			}
			
			void SetTexture(Rendering::Texture* texture)
			{
				this->texture = texture;
			}

		public:
			const FontGlyph* GetGlyph(i32 charCode)
			{
				return getGlyph(charCode);
			}

			void PushGlyph(SpriteRenderer* sprRenderer, const FontGlyph* glyph, const vec2& position, const vec2& scale, const Color& color)
			{
				if (texture == nullptr) { return; }

				float srcX = static_cast<float>(glyph->X);
				float srcY = static_cast<float>(glyph->Y);
				float srcWidth = static_cast<float>(glyph->Width);
				float srcHeight = static_cast<float>(glyph->Height);

				sprRenderer->SetSpritePosition(position);
				sprRenderer->SetSpriteSize(vec2{ srcWidth, srcHeight } *scale);
				sprRenderer->SetSpriteColor(color);
						   
				sprRenderer->SetSpriteSource(texture, RectangleF{ srcX, srcY, srcWidth, srcHeight });
						   
				sprRenderer->SetSpriteRotation(0.0f);
				sprRenderer->SetSpriteOrigin(vec2{ 0.0f, 0.0f });
						   
				sprRenderer->PushSprite(texture);
			}
			
			void PushString(SpriteRenderer* sprRenderer, std::string_view text, const vec2& position, const vec2& scale, const Color& color)
			{
				vec2 basePos{};
				vec2 glyphOffset{};

				auto c = text.cbegin();
				const FontGlyph* prevGlyph{};

				while (c != text.cend())
				{
					i32 utfChar = utf8::next(c, text.cend());

					if (utfChar == '\n')
					{
						basePos.x = 0.0f;
						basePos.y += description.LineHeight;
						prevGlyph = nullptr;
						continue;
					}

					const FontGlyph* glyph = getGlyph(utfChar);

					if (utfChar == ' ')
					{
						basePos.x += glyph->XAdvance;
						prevGlyph = nullptr;
						continue;
					}

					glyphOffset.x = glyph->XOffset;
					glyphOffset.y = glyph->YOffset;

					if (prevGlyph != nullptr && prevGlyph->KerningList != nullptr)
					{
						for (const auto& kern : prevGlyph->KerningList->KerningList)
						{
							if (kern.SecondCharCode == utfChar)
							{ 
								basePos.x += kern.Amount;
							}
						}
					}

					PushGlyph(sprRenderer, glyph, position + basePos + glyphOffset, scale, color);

					basePos.x += glyph->XAdvance;
					prevGlyph = glyph;
				}
			}
		};
	}

	struct FontTest::Impl
	{
		FontTest* parent{};

		std::unique_ptr<Font> testFont_xml{};
		std::unique_ptr<BinFontFormat::NewFont> testFont_bin{};

		static constexpr std::string_view testUtf8String
		{
			"\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x0a\x21\x3f\x2c\x2e\x27" \
			"\x22\x3a\x3b\x40\x23\x24\x25\x5e\x3c\x3e\x26\x2a\x2f\x5c\x7c\x5b" \
			"\x5d\x7b\x7d\x28\x29\x2b\x2d\x3d\x0a\x0a\x54\x48\x45\x20\x51\x55" \
			"\x49\x43\x4b\x20\x42\x52\x4f\x57\x4e\x20\x46\x4f\x58\x20\x4a\x55" \
			"\x4d\x50\x53\x20\x4f\x56\x45\x52\x20\x54\x48\x45\x20\x4c\x41\x5a" \
			"\x59\x20\x44\x4f\x47\x0a\x74\x68\x65\x20\x71\x75\x69\x63\x6b\x20" \
			"\x62\x72\x6f\x77\x6e\x20\x66\x6f\x78\x20\x6a\x75\x6d\x70\x73\x20" \
			"\x6f\x76\x65\x72\x20\x74\x68\x65\x20\x6c\x61\x7a\x79\x20\x64\x6f" \
			"\x67\x0a\x0a\xd0\x92\x20\xd0\xa7\xd0\x90\xd0\xa9\xd0\x90\xd0\xa5" \
			"\x20\xd0\xae\xd0\x93\xd0\x90\x20\xd0\x96\xd0\x98\xd0\x9b\x20\xd0" \
			"\x91\xd0\xab\x20\xd0\xa6\xd0\x98\xd0\xa2\xd0\xa0\xd0\xa3\xd0\xa1" \
			"\x3f\x20\xd0\x94\xd0\x90\x2c\x20\xd0\x9d\xd0\x9e\x20\xd0\xa4\xd0" \
			"\x90\xd0\x9b\xd0\xac\xd0\xa8\xd0\x98\xd0\x92\xd0\xab\xd0\x99\x20" \
			"\xd0\xad\xd0\x9a\xd0\x97\xd0\x95\xd0\x9c\xd0\x9f\xd0\x9b\xd0\xaf" \
			"\xd0\xa0\x21\x20\xd0\x81\xd0\xaa\x0a\xd0\xb2\x20\xd1\x87\xd0\xb0" \
			"\xd1\x89\xd0\xb0\xd1\x85\x20\xd1\x8e\xd0\xb3\xd0\xb0\x20\xd0\xb6" \
			"\xd0\xb8\xd0\xbb\x20\xd0\xb1\xd1\x8b\x20\xd1\x86\xd0\xb8\xd1\x82" \
			"\xd1\x80\xd1\x83\xd1\x81\x3f\x20\xd0\xb4\xd0\xb0\x2c\x20\xd0\xbd" \
			"\xd0\xbe\x20\xd1\x84\xd0\xb0\xd0\xbb\xd1\x8c\xd1\x88\xd0\xb8\xd0" \
			"\xb2\xd1\x8b\xd0\xb9\x20\xd1\x8d\xd0\xba\xd0\xb7\xd0\xb5\xd0\xbc" \
			"\xd0\xbf\xd0\xbb\xd1\x8f\xd1\x80\x21\x20\xd1\x91\xd1\x8a\x0a"
		};

		Impl(FontTest* parent) : parent{ parent }
		{
		}

		~Impl()
		{
		}

		bool LoadContent()
		{
			testFont_xml = std::make_unique<Font>();
			if (!testFont_xml->ReadBMFont("testfiles/fonts/conv_test.fnt")) { return false; }

			testFont_bin = std::make_unique<BinFontFormat::NewFont>();
			IO::FileStream fileStream = IO::File::OpenRead("testfiles/fonts/conv_test.dat");
			IO::StreamReader reader{ fileStream };

			if (!testFont_bin->Read(reader))
			{
				fileStream.Close();
				return false;
			}

			fileStream.Close();

			testFont_bin->SetTexture(testFont_xml->Texture.get());
			return true;
		}

		void UnloadContent()
		{
			testFont_bin = nullptr;
			testFont_xml->Destroy();
			testFont_xml = nullptr;
		}

		void Draw(Starshine::GameTime& gameTime)
		{
			auto gameContext = GameContext::GetInstance();
			auto sprRenderer = gameContext->SpriteRenderer.get();
			auto debugFont = gameContext->DebugFont.get();

			auto gfxDevice = sprRenderer->GetRenderingDevice();

			gfxDevice->Clear(Rendering::ClearFlags_Color, DefaultColors::ClearColor_Menus, 1.0f, 0);
			sprRenderer->SetBlendMode(BlendMode::Normal);

			static constexpr vec2 basePos{ 16.0f, 16.0f };
			vec2 offsetPos{};

			// --- bmfont

			sprRenderer->Font().PushString(debugFont, "BMFont (looping through each glyph):", basePos, vec2{ 1.0f }, DefaultColors::White);

			TimeSpan startTime = TimeSpan::GetTimeNow();
			offsetPos.y += testFont_xml->LineHeight;
			sprRenderer->Font().PushString(testFont_xml.get(), testUtf8String, basePos + offsetPos, vec2{ 1.0f }, DefaultColors::White);
			TimeSpan endTime = TimeSpan::GetTimeNow();

			TimeSpan renderTime_bmfont = endTime - startTime;

			vec2 textSize = sprRenderer->Font().MeasureString(testFont_xml.get(), testUtf8String);

			// --- binary font

			offsetPos.y += testFont_xml->LineHeight + textSize.y;
			sprRenderer->Font().PushString(debugFont, "Binary Font (glyph range mappings):",
				basePos + offsetPos, vec2{ 1.0f }, DefaultColors::White);

			startTime = TimeSpan::GetTimeNow();
			offsetPos.y += testFont_xml->LineHeight;
			testFont_bin->PushString(sprRenderer, testUtf8String, basePos + offsetPos, vec2{ 1.0f }, DefaultColors::White);
			endTime = TimeSpan::GetTimeNow();

			TimeSpan renderTime_binary = endTime - startTime;

			// --- stats
			char statsText[128]{};
			size_t statsTextLength = SDL_snprintf(statsText, sizeof(statsText) - 1, "Stats:\nBMFont - %.3f ms\nBinary Font: %.3f ms",
				renderTime_bmfont.GetMilliseconds(), renderTime_binary.GetMilliseconds());

			offsetPos.y += testFont_xml->LineHeight + textSize.y;
			sprRenderer->Font().PushString(debugFont, statsText, basePos + offsetPos, vec2{ 1.0f }, DefaultColors::White);

			sprRenderer->RenderSprites(nullptr);
		}
	};

	FontTest::FontTest() : impl{ std::make_unique<Impl>(this) }
	{
	}

	FontTest::~FontTest()
	{
	}

	bool FontTest::Initialize()
	{
		return true;
	}

	bool FontTest::LoadContent()
	{
		return impl->LoadContent();
	}

	void FontTest::UnloadContent()
	{
		impl->UnloadContent();
	}

	void FontTest::Destroy()
	{
	}

	void FontTest::Update(Starshine::GameTime& gameTime)
	{
	}

	void FontTest::Draw(Starshine::GameTime& gameTime)
	{
		impl->Draw(gameTime);
	}

	i64 FontTest::GetStateID() const
	{
		return GameState_FontTest;
	}
}
