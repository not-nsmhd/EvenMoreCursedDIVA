#include "string_utils.h"

namespace Utils::String
{
	using std::string_view;

	bool StartsWith(string_view string, string_view checkValue)
	{
		if (string.size() < checkValue.size())
		{
			return false;
		}

		return string.compare(0, string_view::npos, checkValue) == 0;
	}

	bool EndsWith(string_view string, string_view checkValue)
	{
		size_t stringA_size = string.size();
		size_t stringB_size = checkValue.size();

		if (stringA_size < stringB_size)
		{
			return false;
		}

		size_t cmp_StartPos = stringA_size - stringB_size;

		return string.compare(cmp_StartPos, string_view::npos, checkValue) == 0;
	}
}
