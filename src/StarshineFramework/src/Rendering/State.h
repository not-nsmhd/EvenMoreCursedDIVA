#pragma once
#include "Common/Types.h"
#include "GFX/GPUResource.h"
#include "Types.h"

namespace Starshine::Rendering
{
	struct BlendStateDesc
	{
	public:
		BlendStateDesc() {};
		constexpr BlendStateDesc(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha,
			BlendOperation colorOp, BlendOperation alphaOp) : 
			SrcColor{ srcColor }, DstColor{ dstColor }, SrcAlpha{ srcAlpha }, DstAlpha{ dstAlpha },
			ColorOp{ colorOp }, AlphaOp{ alphaOp } {};

	public:
		BlendFactor SrcColor{};
		BlendFactor DstColor{};

		BlendFactor SrcAlpha{};
		BlendFactor DstAlpha{};

		BlendOperation ColorOp{};
		BlendOperation AlphaOp{};
	};

	struct BlendState : public GFX::GPUResource, NonCopyable
	{
	public:
		BlendState() = default;
		BlendState(const BlendStateDesc& desc) : Desc{ desc } {}
		virtual ~BlendState() = default;

		const BlendStateDesc Desc;
	};
}
