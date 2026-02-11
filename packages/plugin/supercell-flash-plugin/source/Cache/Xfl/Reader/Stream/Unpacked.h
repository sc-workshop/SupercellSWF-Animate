#pragma once

#include "Stream.h"

#include <filesystem>
#include "core/memory/ref.h"
#include "core/io/file_stream.h"

#include <mutex>

namespace Animate::IO
{
	class UnpackedStream final : public Stream
	{
	public:
		virtual bool Open(const Path& path);

		virtual bool OpenFile(const Path& path);
		virtual wk::Ref<wk::Stream> ReadFile();
		virtual void CloseFile();

	private:
		std::mutex m_mutex;
		Path m_basedir;
		wk::Ref<wk::Stream> m_active_file;
	};
}