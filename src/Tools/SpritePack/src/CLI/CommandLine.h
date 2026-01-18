#pragma once
#include <Common/Types.h>
#include <vector>
#include <functional>

namespace Starshine::SpritePack::CLI
{
	struct CommandLineOption
	{
		const char* ShortCommand{};
		const char* FullCommand{};
		const char* Description{};

		std::function<void(int index, const char* args[])> Action;

		const char* GetDescription() const;
		bool InputMatches(const char* input) const;

		static bool InputMatches(const char* input, const char* shortForm, const char* fullForm);
	};

	class CommandLineOptions
	{
		CommandLineOptions() = delete;

	public:
		static void Initialize();
		static const std::vector<CommandLineOption>& GetOptions();

	private:
		static std::vector<CommandLineOption> options;
	};
}
