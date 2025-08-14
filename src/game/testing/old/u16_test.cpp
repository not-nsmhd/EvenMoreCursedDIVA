#include "../global_res.h"
#include "u16_test.h"
#include <utf8.h>
#include <fstream>

namespace Testing
{
	static GFX::SpriteRenderer* spriteRenderer;

	bool U16Test::Initialize()
	{
		alphaBlend.srcColor = GFX::LowLevel::BlendFactor::BLEND_SRC_ALPHA;
		alphaBlend.srcAlpha = GFX::LowLevel::BlendFactor::BLEND_SRC_ALPHA;
		alphaBlend.dstColor = GFX::LowLevel::BlendFactor::BLEND_ONE_MINUS_SRC_ALPHA;
		alphaBlend.dstAlpha = GFX::LowLevel::BlendFactor::BLEND_ONE_MINUS_SRC_ALPHA;
		alphaBlend.colorOp = GFX::LowLevel::BlendOp::BLEND_OP_ADD;

		return true;
	}
	
	bool U16Test::LoadContent()
	{
		std::filesystem::path u16test_filePath = fileSystem->GetContentFilePath("resources/u8_test.txt");

		std::fstream u16test_file;
		u16test_file.open(u16test_filePath, std::ios::in | std::ios::binary);

		if (u16test_file.bad())
		{
			return false;
		}

		size_t fileSize = std::filesystem::file_size(u16test_filePath);

		u8* buf = new u8[fileSize + 1];
		u16test_file.read((char*)buf, fileSize);
		buf[fileSize] = 0x00;
		u16test_file.close();

		bool valid = utf8::is_valid(buf, &buf[fileSize - 1]);

		if (!valid)
		{
			delete[] buf;
			return false;
		}

		testTextBuffer = buf;
		testTextBufferSize = fileSize;

		spriteRenderer = GlobalResources::SpriteRenderer;
		font.LoadBMFont(graphicsBackend, "fonts/u16_test.fnt");

		return true;
	}
	
	void U16Test::UnloadContent()
	{
		font.Destroy();
	}
	
	void U16Test::Destroy()
	{
		delete[] testTextBuffer;
	}
	
	void U16Test::OnResize(u32 newWidth, u32 newHeight)
	{
	}
	
	void U16Test::Update()
	{	
	}
	
	void U16Test::Draw()
	{
		graphicsBackend->Clear(GFX::LowLevel::ClearFlags::GFX_CLEAR_COLOR, Common::Color(0, 24, 24, 255), 1.0f, 0);

		font.PushUTF8String(spriteRenderer, testTextBuffer, testTextBufferSize, vec2(16.0f), vec2(1.0f), Common::DefaultColors::White);

		graphicsBackend->SetBlendState(&alphaBlend);
		spriteRenderer->RenderSprites(nullptr);

		graphicsBackend->SwapBuffers();
	}
}