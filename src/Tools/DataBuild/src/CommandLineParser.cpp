#include "CommandLineParser.h"
#include "Common/Logging/Logging.h"
#include <SDL2/SDL_stdinc.h>
#include <vector>

namespace Starshine::CLI
{
	namespace
	{
		struct CommandLineOptionValue
		{
			CommandLineOptionDesc Desc{};

			// NOTE: string length for non-empty options, 1 for empty options is argument is present
			size_t ValueSize{};
			const char* Value{};
		};

		static std::vector<CommandLineOptionValue> optionValues;

		const CommandLineOptionValue* TryGetOptionValue(const char* fullName)
		{
			for (const auto& it : optionValues)
			{
				if (SDL_strcmp(it.Desc.FullName, fullName) == 0)
					return &it;
			}

			return nullptr;
		}

		bool MatchesCommand(const char* argument, const char* fullName, const char* shortName)
		{
			if (shortName == nullptr)
				return SDL_strcmp(argument, fullName) == 0;

			return (SDL_strcmp(argument, fullName) == 0) || (SDL_strcmp(argument, shortName) == 0);
		}
	}

	ParseResult CommandLineParser::Parse(i32 count, char* arguments[])
	{
		i32 currentArgumentIndex = 0;
		i32 remainingArguments = count - currentArgumentIndex;

		if (remainingArguments > 0)
		{
			if (MatchesCommand(arguments[0], "--help", "-h"))
			{
				LogMessage("Options:");

				for (const auto& option : optionValues)
				{
					bool empty = option.Desc.IsEmpty;
					if (option.Desc.ShortName != nullptr)
					{
						LogMessage(empty ? "\t%s | %s \t%s" : "\t%s | %s value \t%s",
							option.Desc.ShortName, option.Desc.FullName, option.Desc.Description);
					}
					else
						LogMessage(empty ? "\t%s \t%s" : "\t%s value \t%s", option.Desc.FullName, option.Desc.Description);
				}

				return ParseResult::Exit;
			}
		}

		while (remainingArguments > 0)
		{
			for (auto& option : optionValues)
			{
				if (MatchesCommand(arguments[currentArgumentIndex], option.Desc.FullName, option.Desc.ShortName))
				{
					if (!option.Desc.IsEmpty)
					{
						const char* argValue = arguments[currentArgumentIndex + 1];
						option.ValueSize = SDL_strlen(argValue);
						option.Value = argValue;

						currentArgumentIndex++;
						remainingArguments--;
						
					}
					else
						option.ValueSize = 1;

					goto NextArg;
				}
			}

		NextArg:
			currentArgumentIndex++;
			remainingArguments--;
		}

		return ParseResult::Continue;
	}

	void CommandLineParser::AddOption(const CommandLineOptionDesc& desc)
	{
		CommandLineOptionValue& optionValue = optionValues.emplace_back();
		optionValue.Desc = desc;
	}

	bool CommandLineParser::TryGetValue(const CommandLineOptionDesc& desc, i32& value)
	{
		const CommandLineOptionValue* option = TryGetOptionValue(desc.FullName);
		if (option == nullptr)
			return false;

		if (option->ValueSize == 0)
			return false;

		value = SDL_atoi(option->Value);

		return true;
	}

	bool CommandLineParser::TryGetValue(const CommandLineOptionDesc& desc, f32& value)
	{
		const CommandLineOptionValue* option = TryGetOptionValue(desc.FullName);
		if (option == nullptr)
			return false;

		if (option->ValueSize == 0)
			return false;

		value = SDL_atof(option->Value);

		return true;
	}

	bool CommandLineParser::TryGetValue(const CommandLineOptionDesc& desc, std::string& value)
	{
		const CommandLineOptionValue* option = TryGetOptionValue(desc.FullName);
		if (option == nullptr)
			return false;

		if (option->ValueSize == 0)
			return false;

		value.reserve(option->ValueSize);
		value.assign(option->Value, option->ValueSize);

		return true;
	}

	bool CommandLineParser::IsOptionPresent(const CommandLineOptionDesc& desc)
	{
		const CommandLineOptionValue* option = TryGetOptionValue(desc.FullName);
		if (option == nullptr)
			return false;

		return option->ValueSize == 1;
	}
}
