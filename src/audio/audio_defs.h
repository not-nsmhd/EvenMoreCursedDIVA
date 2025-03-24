#pragma once
#include "util/logging.h"

#define LOG_INFO(message) Logging::LogInfo("Audio", message)
#define LOG_INFO_ARGS(message, args...) Logging::LogInfo("Audio", message, args)
#define LOG_WARN(message) Logging::LogWarn("Audio", message)
#define LOG_WARN_ARGS(message, args...) Logging::LogWarn("Audio", message, args)
#define LOG_ERROR(message) Logging::LogError("Audio", message)
#define LOG_ERROR_ARGS(message, args...) Logging::LogError("Audio", message, args)
