#pragma once
#include <string>

namespace Utils::String
{
	bool StartsWith(std::string_view string, std::string_view checkValue);
	bool EndsWith(std::string_view string, std::string_view checkValue);
}
