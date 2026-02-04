#include "Device.h"
#include "OpenGL/OpenGLDevice.h"
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
			GlobalDevice = std::make_unique<OpenGL::OpenGLDevice>();
			break;
		case DeviceType::D3D9:
			assert(("Direct3D 9 rendering device is not implemented yet", false));
			return false;
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
