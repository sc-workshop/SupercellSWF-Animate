#include "Stream.h"

namespace Animate::IO {
	wk::Ref<wk::Stream> Stream::Read(const Path& path) {
		if (!OpenFile(path))
		{
			throw wk::Exception("Failed to open file for writing: " + path.string());
		}

		const auto result = ReadFile();
		if (!result)
		{
			throw wk::Exception("Failed to write data to file: " + path.string());
		}

		CloseFile();
		return result;
	}
}