#include "common/types.h"

namespace DIVA::Audio
{
	struct Voice
	{
		struct Impl;
		Impl* impl = nullptr;
	};

	class AudioEngine : NonCopyable
	{
	public:
		AudioEngine();
		~AudioEngine();

	private:
		struct Impl;
		Impl* impl = nullptr;
	};
}
