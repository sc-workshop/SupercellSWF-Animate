#pragma once

#include "Stream.h"

#include "core/io/memory_stream.h"
#include "core/asset_manager/asset_manager.h"

#include <mutex>

struct zip_t;

namespace Animate::IO
{
	class PackedStream final : public Stream
	{
	public:
		virtual ~PackedStream();

	public:
		virtual bool Open(const Path& path);

		virtual bool OpenFile(const Path& path);
		virtual wk::Ref<wk::Stream> ReadFile() = 0;
		virtual void CloseFile();

	private:
		std::mutex m_mutex;
		wk::Ref<wk::Stream> m_file;
		zip_t* m_context = nullptr;
	};
}