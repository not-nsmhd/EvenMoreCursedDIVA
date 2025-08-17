#pragma once
#include "../../common/color.h"

namespace GFX
{
	namespace LowLevel
	{
		enum class BlendFactor
		{
			BLEND_ZERO,
			BLEND_ONE,

			BLEND_SRC_COLOR,
			BLEND_ONE_MINUS_SRC_COLOR,
			BLEND_DST_COLOR,
			BLEND_ONE_MINUS_DST_COLOR,

			BLEND_SRC_ALPHA,
			BLEND_ONE_MINUS_SRC_ALPHA,
			BLEND_DST_ALPHA,
			BLEND_ONE_MINUS_DST_ALPHA,

			BLEND_CONSTANT_COLOR,
			BLEND_ONE_MINUS_CONSTANT_COLOR,

			BLEND_CONSTANT_ALPHA,
			BLEND_ONE_MINUS_CONSTANT_ALPHA
		};

		enum class BlendOp
		{
			BLEND_OP_ADD,
			BLEND_OP_SUBTRACT,
			BLEND_OP_REVERSE_SUBTRACT,
			BLEND_OP_MIN,
			BLEND_OP_MAX
		};

		struct BlendState
		{
			BlendFactor srcColor;
			BlendFactor dstColor;

			BlendFactor srcAlpha;
			BlendFactor dstAlpha;

			BlendOp colorOp;

			Common::Color constantColor;
		};

		namespace DefaultBlendStates
		{
			const BlendState Opaque = 
			{ 
				BlendFactor::BLEND_ONE, 
				BlendFactor::BLEND_ZERO, 
				BlendFactor::BLEND_ONE, 
				BlendFactor::BLEND_ZERO, 
				BlendOp::BLEND_OP_ADD 
			};

			const BlendState AlphaBlend = 
			{ 
				BlendFactor::BLEND_SRC_ALPHA, 
				BlendFactor::BLEND_ONE_MINUS_SRC_ALPHA, 
				BlendFactor::BLEND_SRC_ALPHA, 
				BlendFactor::BLEND_ONE_MINUS_SRC_ALPHA, 
				BlendOp::BLEND_OP_ADD 
			};
		}
	}
}
