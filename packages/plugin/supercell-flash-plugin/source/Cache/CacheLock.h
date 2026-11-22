#pragma once

#include <filesystem>
#include <memory>

namespace sc::Adobe::Cache {
    class CacheLock {
    public:
        CacheLock();
        ~CacheLock();

        CacheLock(const CacheLock&) = delete;
        CacheLock& operator=(const CacheLock&) = delete;

        bool TryLock(const std::filesystem::path& path);
        void Unlock();

        bool IsLocked() const { return m_locked; }

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
        bool m_locked = false;
    };
}
