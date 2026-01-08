#pragma once
#include "common/types.h"

namespace Starshine::Audio
{
	class ISampleProvider
	{
	public:
		virtual ~ISampleProvider() = default;
		virtual void Destroy() = 0;

		virtual bool IsStreamingOnly() const = 0;

		virtual u32 GetChannelCount() const = 0;
		virtual u32 GetSampleRate() const = 0;
		virtual size_t GetSampleAmount() const = 0;

		virtual size_t GetLoopStart_Frames() const = 0;
		virtual size_t GetLoopEnd_Frames() const = 0;

		virtual size_t ReadSamples(i16* dstBuffer, size_t offset, size_t size) = 0;

	public:
		virtual size_t GetSamplePosition() const = 0;

		virtual size_t GetNextSamples(i16* dstBuffer, size_t size) = 0;
		virtual void Seek(size_t samplePosition) = 0;
	};
}
