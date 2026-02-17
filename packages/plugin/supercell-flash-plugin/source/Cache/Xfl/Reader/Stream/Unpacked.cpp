#include "Unpacked.h"

#include "assert.h"

namespace fs = std::filesystem;

namespace Animate::IO {
    bool UnpackedStream::Open(const Path& path) {
        if (!fs::exists(path))
            return false;

        m_basedir = path;
        for (const auto& entry : fs::recursive_directory_iterator(m_basedir)) {
            if (!entry.is_regular_file())
                continue;

            auto name = std::filesystem::relative(m_basedir, entry.path());
            m_paths.push_back(name);
        }

        return true;
    }

    bool UnpackedStream::OpenFile(const Path& path) {
        fs::path destination = m_basedir;
        destination /= path;

        m_mutex.lock();
        m_active_file = wk::CreateRef<wk::InputFileStream>(destination);
        return m_active_file->is_open();
    }

    wk::Ref<wk::Stream> UnpackedStream::ReadFile() {
        return m_active_file;
    }

    void UnpackedStream::CloseFile() {
        m_active_file.reset();
        m_mutex.unlock();
    }
}