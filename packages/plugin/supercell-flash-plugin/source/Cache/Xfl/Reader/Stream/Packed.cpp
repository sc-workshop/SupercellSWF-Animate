#include "Packed.h"

#include "zip.h"
#include "assert.h"

#include <algorithm>

namespace fs = std::filesystem;

namespace Animate::IO
{
	PackedStream::~PackedStream()
	{
		if (m_context)
		{
			zip_close(m_context);
		}
	}

	bool PackedStream::Open(const Path& path)
	{
		auto& manager = wk::AssetManager::Instance();
		int error = 0;

		m_file = manager.load_file(path);
		m_context = zip_stream_openwitherror(
			(const char*)m_file->data(), m_file->length(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'r', &error
		);

		if (error != 0)
			return false;

		size_t count = zip_entries_total(m_context);
		for (size_t i = 0; count > i; i++) {
			zip_entry_openbyindex(m_context, i);
			if (!zip_entry_isdir(m_context)) {
				Path path = zip_entry_name(m_context);
				m_paths.push_back(path);
			}

			zip_entry_close(m_context);
		}

		return true;
	}

	bool PackedStream::OpenFile(const Path& path)
	{
		m_mutex.lock();
		auto code = zip_entry_open(m_context, path.string().c_str());
		assert(code == 0 && "Failed to open zip entry");
		return code == 0;
	}

	wk::Ref<wk::Stream> PackedStream::ReadFile() {
		size_t size = zip_entry_size(m_context);

		auto stream = wk::CreateRef<wk::MemoryStream>(size);
		auto result = zip_entry_noallocread(m_context, stream->data(), size);
		if (result != 0)
			return nullptr;

		return stream;
	}

	void PackedStream::CloseFile()
	{
		zip_entry_close(m_context);
		m_mutex.unlock();
	}
}