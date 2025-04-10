#pragma once
#include "../util/logging.h"

#define LOG_INFO(message) Logging::LogInfo("Audio", message)
#define LOG_INFO_ARGS(message, ...) Logging::LogInfo("Audio", message, __VA_ARGS__)
#define LOG_WARN(message) Logging::LogWarn("Audio", message)
#define LOG_WARN_ARGS(message, ...) Logging::LogWarn("Audio", message, __VA_ARGS__)
#define LOG_ERROR(message) Logging::LogError("Audio", message)
#define LOG_ERROR_ARGS(message, ...) Logging::LogError("Audio", message, __VA_ARGS__)
