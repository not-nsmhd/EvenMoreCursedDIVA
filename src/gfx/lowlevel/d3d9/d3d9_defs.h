#pragma once
#include "../../../util/logging.h"

#define LOG_INFO(message) Logging::LogInfo("GFX::D3D9", message)
#define LOG_INFO_ARGS(message, ...) Logging::LogInfo("GFX::D3D9", message, __VA_ARGS__)
#define LOG_WARN(message) Logging::LogWarn("GFX::D3D9", message)
#define LOG_WARN_ARGS(message, ...) Logging::LogWarn("GFX::D3D9", message, __VA_ARGS__)
#define LOG_ERROR(message) Logging::LogError("GFX::D3D9", message)
#define LOG_ERROR_ARGS(message, ...) Logging::LogError("GFX::D3D9", message, __VA_ARGS__)
