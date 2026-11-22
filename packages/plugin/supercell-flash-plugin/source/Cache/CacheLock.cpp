#include "CacheLock.h"

#include <system_error>

#if defined(_WIN32) || defined(_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace sc::Adobe::Cache {
#if defined(_WIN32) || defined(_WINDOWS)
    struct CacheLock::Impl {
        HANDLE handle = INVALID_HANDLE_VALUE;
    };
#else
    struct CacheLock::Impl {
        int descriptor = -1;
    };
#endif

    CacheLock::CacheLock() : m_impl(std::make_unique<Impl>()) {}

    CacheLock::~CacheLock() {
        Unlock();
    }

    bool CacheLock::TryLock(const std::filesystem::path& path) {
        Unlock();

#if defined(_WIN32) || defined(_WINDOWS)
        m_impl->handle = CreateFileW(
            path.wstring().c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (m_impl->handle == INVALID_HANDLE_VALUE) {
            return false;
        }

        OVERLAPPED overlapped{};
        m_locked = LockFileEx(
                       m_impl->handle,
                       LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
                       0,
                       1,
                       0,
                       &overlapped) != FALSE;
        if (!m_locked) {
            CloseHandle(m_impl->handle);
            m_impl->handle = INVALID_HANDLE_VALUE;
        }
#else
        m_impl->descriptor = open(path.c_str(), O_CREAT | O_RDWR, 0666);
        if (m_impl->descriptor < 0) {
            return false;
        }

        m_locked = flock(m_impl->descriptor, LOCK_EX | LOCK_NB) == 0;
        if (!m_locked) {
            close(m_impl->descriptor);
            m_impl->descriptor = -1;
        }
#endif

        return m_locked;
    }

    void CacheLock::Unlock() {
        if (!m_locked)
            return;

#if defined(_WIN32) || defined(_WINDOWS)
        if (m_impl->handle != INVALID_HANDLE_VALUE) {
            OVERLAPPED overlapped{};
            UnlockFileEx(m_impl->handle, 0, 1, 0, &overlapped);
            CloseHandle(m_impl->handle);
            m_impl->handle = INVALID_HANDLE_VALUE;
        }
#else
        if (m_impl->descriptor >= 0) {
            flock(m_impl->descriptor, LOCK_UN);
            close(m_impl->descriptor);
            m_impl->descriptor = -1;
        }
#endif

        m_locked = false;
    }
}
