#pragma once
#include <Common/Types.h>
#include <TimeSpan.h>
#include <Graphics/Texture.h>
#include <memory>

namespace Starshine::Video
{
	class VideoPlayer : NonCopyable
	{
	public:
		VideoPlayer() = default;
		virtual ~VideoPlayer() = default;

	public:
		virtual bool OpenFile(std::string_view filePath) = 0;
		virtual void CloseFile() = 0;

		virtual bool GetIsPlaying() = 0;
		virtual void SetIsPlaying(bool playing) = 0;

		virtual TimeSpan GetDuration() = 0;
		virtual TimeSpan GetPosition() = 0;

		virtual ivec2 GetResolution() = 0;

		virtual Graphics::Texture* GetVideoTexture() = 0;
	};

	std::unique_ptr<VideoPlayer> InitializeVideoPlayer_MediaFoundation();
}
