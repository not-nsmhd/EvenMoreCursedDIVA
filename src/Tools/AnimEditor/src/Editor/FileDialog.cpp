#include "FileDialog.h"
#include <Windows.h>
#include <ShObjIdl.h>
#include <wrl.h>
#include <utf8.h>

using namespace Microsoft::WRL;

namespace Starshine
{
	bool FileDialog::OpenRead()
	{
		return InternalOpenDialog(false);
	}

	bool FileDialog::OpenSave()
	{
		return InternalOpenDialog(true);
	}

	bool FileDialog::InternalOpenDialog(bool save)
	{
		HRESULT result = S_OK;
		ComPtr<IFileDialog> fileDialog = nullptr;

		const IID dialogClassID = save ? CLSID_FileSaveDialog : CLSID_FileOpenDialog;
		const IID dialogInterfaceID = save ? IID_IFileSaveDialog : IID_IFileOpenDialog;

		if (result = CoCreateInstance(dialogClassID, NULL, CLSCTX_ALL, dialogInterfaceID, (void**)fileDialog.GetAddressOf()), result != S_OK)
			return false;

		if (!Title.empty())
		{
			static std::array<WCHAR, 256> titleBuffer{};
			utf8::utf8to16(Title.data(), Title.data() + Title.size(), titleBuffer.data());

			result = fileDialog->SetTitle(titleBuffer.data());
		}

		if (result = fileDialog->Show(NULL), result == S_OK)
		{
			ComPtr<IShellItem> shellItem = nullptr;
			if (result = fileDialog->GetResult(shellItem.GetAddressOf()), result == S_OK)
			{
				WCHAR* fileName{};
				if (result = shellItem->GetDisplayName(SIGDN_FILESYSPATH, &fileName), result == S_OK)
				{
					static std::array<char, 256> fileNameBuffer{};
					offset_t length = static_cast<offset_t>(lstrlenW(fileName));

					utf8::utf16to8(fileName, fileName + length, fileNameBuffer.data());
					OutputFilePath = std::string(fileNameBuffer.data());

					CoTaskMemFree(fileName);
				}
			}
		}

		return (result == S_OK) && (!OutputFilePath.empty());
	}
}
