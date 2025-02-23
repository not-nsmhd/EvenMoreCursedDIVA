#pragma once
#include <vector>
#include <string>

namespace Utils
{
	namespace String
	{
		bool StartsWith(std::string string, std::string checkValue);
		bool EndsWith(std::string string, std::string checkValue);

		std::vector<std::string> Split(std::string string, std::string separator);
	};
};
