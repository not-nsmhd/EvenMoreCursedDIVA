#pragma once
#include "../../common/int_types.h"

namespace GFX
{
	namespace LowLevel
	{
		class Shader
		{
		public:
			Shader() {}

			virtual const char* GetDebugName() const = 0;
			virtual void SetDebugName(const char* name) = 0;
		protected:
			char* debugName = nullptr;
		};
	}
}