#pragma once

#include "core/memory/ref.h"
#include "core/exception/exception.h"
#include <filesystem>

#include "Stream/Packed.h"
#include "Stream/Unpacked.h"

namespace Animate::XFL
{
	using namespace wk;

	class XFLFile
	{
	private:
		enum class XFLType
		{
			Unpacked,
			Packed
		};

	public:
		XFLFile(const Path& path);

	public:
		const std::vector<Path>& GetPaths();
		wk::Ref<wk::Stream> ReadFile(const Path& path);

	private:
		void CreateStream();

	private:
		Ref<IO::Stream> m_stream;
		XFLType m_filetype;
		Path m_document_path;
	};
}