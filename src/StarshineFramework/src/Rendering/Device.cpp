#include "Device.h"
#include "D3D11/D3D11Device.h"
#include "Common/Logging/Logging.h"

namespace Starshine::Rendering
{
	std::unique_ptr<Device> GlobalDevice{};
	DeviceType GlobalDeviceType{};

	constexpr const char* LogName = "Starshine::Rendering";

	bool InitializeDevice(SDL_Window* sdlWindow, DeviceType type)
	{
		if (sdlWindow == nullptr) { return false; }

		switch (type)
		{
		case DeviceType::OpenGL:
			assert(false && "OpenGL rendering device is not implemented yet");
			break;
		case DeviceType::D3D11:
			GlobalDevice = std::make_unique<D3D11::D3D11Device>();
			break;
		}

		LogInfo(LogName, "Device Type: %s", DeviceTypeNames[static_cast<size_t>(type)]);
		GlobalDeviceType = type;

		return GlobalDevice->Initialize(sdlWindow);
	}

	void DestroyDevice()
	{
		if (GlobalDevice != nullptr)
		{
			GlobalDevice->Destroy();
			GlobalDevice = nullptr;
		}
	}

	Device* GetDevice()
	{
		return GlobalDevice.get();
	}

	DeviceType GetDeviceType()
	{
		return GlobalDeviceType;
	}
}
