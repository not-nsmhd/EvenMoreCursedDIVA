#pragma once

namespace GFX::LowLevel
{
	class Resource
	{
	public:
		const char* GetDebugName();
		void SetDebugName(const char* name);
	protected:
		char name[128] = {};
		bool nameSet = false;
	};
}
