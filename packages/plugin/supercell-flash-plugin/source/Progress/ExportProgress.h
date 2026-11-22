#pragma once

#include "core/exception/exception.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>

namespace sc::Adobe {
    enum class ExportPhase : uint8_t {
        Preparing,
        LoadingDocuments,
        ExternalFile,
        Extracting,
        Committing,
        Atlas,
        Saving,
        Done,
        Failed,
        Cancelled
    };

    enum class ExportOutcome : uint8_t {
        None,
        Success,
        Failure,
        Cancelled
    };

    struct ProgressSnapshot {
        ExportPhase phase = ExportPhase::Preparing;
        ExportOutcome outcome = ExportOutcome::None;
        size_t document = 0;
        size_t documentCount = 0;
        std::string documentName;
        std::string detail;
        std::string subdetail;
        std::string error;
        size_t done = 0;
        size_t total = 0;
        double elapsedSeconds = 0.0;
        bool cancelling = false;
    };

    /// @brief Thread-safe publish state shared by the export worker and the progress UI.
    ///
    /// No UI framework types live here. Worker threads only mutate counters and strings
    /// the UI thread polls a snapshot once per frame.
    /// Cancellation is cooperative: workers must call
    /// ThrowIfCancelled()/IsCancelled() at well-defined safe points.
    class ExportProgress {
    public:
        void Reset(size_t documentCount = 0);

        void SetPhase(ExportPhase phase);
        void BeginPhase(ExportPhase phase, size_t total = 0, std::string detail = {});
        void SetDocuments(size_t current, size_t count, std::string name = {});
        void SetCounts(size_t done, size_t total);
        void Advance(size_t amount = 1);
        void SetDetail(std::string detail);
        void SetSubdetail(std::string detail);
        void SetError(std::string error);
        void Finish(ExportOutcome outcome, std::string error = {});

        void RequestCancel();
        bool IsCancelled() const { return m_cancel.load(std::memory_order_acquire); }
        bool IsFinished() const { return m_finished.load(std::memory_order_acquire); }

        ProgressSnapshot Snapshot() const;

    private:
        mutable std::mutex m_mutex;
        ProgressSnapshot m_snapshot;
        std::chrono::steady_clock::time_point m_start = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point m_end;
        bool m_endSet = false;
        std::atomic<bool> m_cancel {false};
        std::atomic<bool> m_finished {false};
    };

    class ExportCancelled : public wk::Exception {
    public:
        ExportCancelled() :
            wk::Exception("Publishing cancelled") {}
    };

    inline void ThrowIfCancelled(const ExportProgress* progress) {
        if (progress && progress->IsCancelled())
            throw ExportCancelled();
    }
}
