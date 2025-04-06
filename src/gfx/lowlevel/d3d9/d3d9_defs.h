#pragma once
#include "util/logging.h"

#define LOG_INFO(message) Logging::LogInfo("GFX::D3D9", message)
#define LOG_INFO_ARGS(message, args...) Logging::LogInfo("GFX::D3D9", message, args)
#define LOG_WARN(message) Logging::LogWarn("GFX::D3D9", message)
#define LOG_WARN_ARGS(message, args...) Logging::LogWarn("GFX::D3D9", message, args)
#define LOG_ERROR(message) Logging::LogError("GFX::D3D9", message)
#define LOG_ERROR_ARGS(message, args...) Logging::LogError("GFX::D3D9", message, args)
