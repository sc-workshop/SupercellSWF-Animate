#include "File.h"

namespace fs = std::filesystem;

namespace Animate::XFL
{
	XFLFile::XFLFile(const Path & path)
	{
		m_filetype = path.extension() == ".xfl" ? XFLType::Unpacked : XFLType::Packed;

		if (m_filetype == XFLType::Unpacked)
		{
			m_document_path = path;
			m_document_path.replace_extension("");
		}
		else
		{
			m_document_path = path;
		}

		CreateStream();
	}

	void XFLFile::CreateStream()
	{
		if (m_filetype == XFLType::Packed)
		{
			m_stream = CreateRef<IO::PackedStream>();
		}
		else
		{
			m_stream = CreateRef<IO::UnpackedStream>();
		}

		if (!m_stream->Open(m_document_path))
		{
			throw Exception("Failed to open XFL file %s", m_document_path.c_str());
		}
	}
}