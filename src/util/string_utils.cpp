#include "string_utils.h"

namespace Utils
{
	namespace String
	{
		bool StartsWith(std::string string, std::string checkValue)
		{
			if (string.size() < checkValue.size())
			{
				return false;
			}

			return string.compare(0, std::string::npos, checkValue) == 0;
		}

		bool EndsWith(std::string string, std::string checkValue)
		{
			size_t stringA_size = string.size();
			size_t stringB_size = checkValue.size();

			if (stringA_size < stringB_size)
			{
				return false;
			}

			size_t cmp_StartPos = stringA_size - stringB_size;

			return string.compare(cmp_StartPos, std::string::npos, checkValue) == 0;
		}

		std::vector<std::string> Split(std::string string, std::string separator)
		{
			if (string.length() <= separator.length())
			{
				return std::vector<std::string>(1, string);
			}

			std::vector<std::string> self;
			size_t splitOffset = 0;
			size_t searchOffset = 0;

			while (true)
			{
				searchOffset = string.find(separator, searchOffset);

				if (searchOffset == std::string::npos)
				{
					break;
				}

				self.push_back(string.substr(splitOffset, searchOffset - splitOffset));

				searchOffset += separator.length();
				splitOffset += searchOffset - splitOffset;
			}

			self.push_back(string.substr(splitOffset));

			return self;
		}
	};
};
