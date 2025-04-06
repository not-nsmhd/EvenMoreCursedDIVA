#pragma once
#include "common/int_types.h"
#include "resource.h"

namespace GFX::LowLevel
{
	class Shader : public Resource
	{
	public:
		Shader() {}

		virtual void Destroy() = 0;
	};
}
