#include "XmlStreamPrinter.h"
#include <cstdarg>
#include <SDL2/SDL_stdinc.h>

namespace Starshine::Xml
{
	StreamPrinter::StreamPrinter(IO::IStream& outputStream)
		: stream(outputStream)
	{
		assert(stream.IsWritable());
	}

	StreamPrinter::~StreamPrinter()
	{
	}

	void StreamPrinter::PrintSpace(int depth)
	{
		for (i32 i = 0; i < depth; i++)
			Print("\t");
	}

	void StreamPrinter::Print(const char* format, ...)
	{
		va_list list;
		va_start(list, format);

		char buffer[256]{};
		int size = SDL_vsnprintf(buffer, sizeof(buffer), format, list);
		assert(size >= 0);
		Write(buffer, size);

		va_end(list);
	}

	void StreamPrinter::Write(const char* data, size_t size)
	{
		stream.WriteBuffer(data, size);
	}

	void StreamPrinter::Putc(char ch)
	{
		stream.WriteBuffer(&ch, sizeof(char));
	}
}
