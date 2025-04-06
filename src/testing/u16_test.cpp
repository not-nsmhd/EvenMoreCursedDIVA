#include "u16_test.h"
#include <fstream>

namespace Testing
{
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
		std::filesystem::path u16test_filePath = fileSystem->GetContentFilePath("resources/u16_test.txt");

		std::fstream u16test_file;
		u16test_file.open(u16test_filePath, std::ios::in | std::ios::binary);

		if (u16test_file.bad())
		{
			return false;
		}

		size_t fileSize = std::filesystem::file_size(u16test_filePath);

		char16_t* buf = new char16_t[(fileSize / 2) + 1];
		u16test_file.read((char*)buf, fileSize);
		buf[fileSize / 2] = 0x0000;
		u16test_file.close();

		testText = u16string(buf);
		delete[] buf;

		spriteRenderer.Initialize(graphicsBackend);
		font.LoadBMFont(graphicsBackend, "fonts/u16_test.fnt");

		return true;
	}
	
	void U16Test::UnloadContent()
	{
		spriteRenderer.Destroy();
		font.Destroy();
	}
	
	void U16Test::Destroy()
	{
		testText.clear();
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

		font.PushString(spriteRenderer, testText, vec2(16.0f), vec2(1.0f), Common::DefaultColors::White);

		graphicsBackend->SetBlendState(&alphaBlend);
		spriteRenderer.RenderSprites(nullptr);

		graphicsBackend->SwapBuffers();
	}
}