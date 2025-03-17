#pragma once
#include "util/logging.h"

#define LOG_INFO(message) Logging::LogInfo("GFX::OpenGL_ARB", message)
#define LOG_INFO_ARGS(message, args...) Logging::LogInfo("GFX::OpenGL_ARB", message, args)
#define LOG_WARN(message) Logging::LogWarn("GFX::OpenGL_ARB", message)
#define LOG_WARN_ARGS(message, args...) Logging::LogWarn("GFX::OpenGL_ARB", message, args)
#define LOG_ERROR(message) Logging::LogError("GFX::OpenGL_ARB", message)
#define LOG_ERROR_ARGS(message, args...) Logging::LogError("GFX::OpenGL_ARB", message, args)
