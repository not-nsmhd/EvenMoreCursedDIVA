#pragma once
#include "Common/Types.h"
#include "IO/IStream.h"
#include <tinyxml2.h>

namespace Starshine::Xml
{
	class StreamPrinter : public tinyxml2::XMLPrinter, NonCopyable
	{
	public:
		StreamPrinter(IO::IStream& outputStream);
		~StreamPrinter() override;

	public:
		void PrintSpace(int depth) override;
		void Print(const char* format, ...) override;
		void Write(const char* data, size_t size) override;
		void Putc(char ch) override;

	private:
		IO::IStream& stream;
	};
}
