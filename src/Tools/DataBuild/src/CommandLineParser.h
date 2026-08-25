#pragma once
#include <Common/Types.h>
#include <string>

namespace Starshine::CLI
{
	struct CommandLineOptionDesc
	{
		const char* FullName{}; // NOTE: Must always be present
		const char* ShortName{};

		const char* Description{};
		bool IsEmpty{};

		CommandLineOptionDesc() {};
		constexpr CommandLineOptionDesc(const char* fullName, const char* shortName, const char* desc, bool isEmpty = false)
			: FullName(fullName), ShortName(shortName), Description(desc), IsEmpty(isEmpty) {}
	};

	enum class ParseResult
	{
		Continue,
		Exit
	};

	class CommandLineParser
	{
		CommandLineParser() = delete;

	public:
		static ParseResult Parse(i32 count, char* arguments[]);

		static void AddOption(const CommandLineOptionDesc& desc);

		static bool TryGetValue(const CommandLineOptionDesc& desc, i32& value);
		static bool TryGetValue(const CommandLineOptionDesc& desc, f32& value);
		static bool TryGetValue(const CommandLineOptionDesc& desc, std::string& value);
		static bool IsOptionPresent(const CommandLineOptionDesc& desc);
	};
}
