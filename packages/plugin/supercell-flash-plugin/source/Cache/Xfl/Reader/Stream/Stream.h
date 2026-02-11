#pragma once

#include "core/exception/exception.h"
#include "core/memory/ref.h"
#include "core/io/stream.h"
#include <filesystem>

namespace Animate {
	using Path = std::filesystem::path;
}

namespace Animate::IO
{
	class Stream
	{
	public:
		virtual ~Stream() = default;

	public:
		virtual bool Open(const Path& path) = 0;
		virtual wk::Ref<wk::Stream> Read(const Path& path);

		virtual bool OpenFile(const Path& path) = 0;
		virtual wk::Ref<wk::Stream> ReadFile() = 0;
		virtual void CloseFile() = 0;

	public:
		const std::vector<Path>& GetPaths() const { return m_paths; }

	protected:
		std::vector<Path> m_paths;
	};
}