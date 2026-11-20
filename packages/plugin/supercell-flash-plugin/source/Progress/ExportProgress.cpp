#include "ExportProgress.h"

namespace sc::Adobe {
    void ExportProgress::Reset(size_t documentCount) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_snapshot = ProgressSnapshot();
        m_snapshot.documentCount = documentCount;
        m_start = std::chrono::steady_clock::now();
        m_endSet = false;
        m_cancel.store(false, std::memory_order_release);
        m_finished.store(false, std::memory_order_release);
    }

    void ExportProgress::SetPhase(ExportPhase phase) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_snapshot.phase = phase;
    }

    void ExportProgress::BeginPhase(ExportPhase phase, size_t total, std::string detail) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_snapshot.phase = phase;
        m_snapshot.done = 0;
        m_snapshot.total = total;
        m_snapshot.detail = std::move(detail);
        m_snapshot.subdetail.clear();
    }

    void ExportProgress::SetDocuments(size_t current, size_t count, std::string name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_snapshot.document = current;
        m_snapshot.documentCount = count;
        m_snapshot.documentName = std::move(name);
    }

    void ExportProgress::SetCounts(size_t done, size_t total) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_snapshot.done = done;
        m_snapshot.total = total;
    }

    void ExportProgress::Advance(size_t amount) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_snapshot.done += amount;
        if (m_snapshot.total && m_snapshot.done > m_snapshot.total)
            m_snapshot.done = m_snapshot.total;
    }

    void ExportProgress::SetDetail(std::string detail) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_snapshot.detail = std::move(detail);
    }

    void ExportProgress::SetSubdetail(std::string detail) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_snapshot.subdetail = std::move(detail);
    }

    void ExportProgress::SetError(std::string error) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_snapshot.error = std::move(error);
    }

    void ExportProgress::Finish(ExportOutcome outcome, std::string error) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_snapshot.outcome = outcome;
            m_snapshot.error = std::move(error);
            m_snapshot.cancelling = false;
            m_end = std::chrono::steady_clock::now();
            m_endSet = true;

            switch (outcome) {
                case ExportOutcome::Success:
                    m_snapshot.phase = ExportPhase::Done;
                    break;
                case ExportOutcome::Failure:
                    m_snapshot.phase = ExportPhase::Failed;
                    break;
                case ExportOutcome::Cancelled:
                    m_snapshot.phase = ExportPhase::Cancelled;
                    break;
                default:
                    break;
            }
        }

        m_finished.store(true, std::memory_order_release);
    }

    void ExportProgress::RequestCancel() {
        m_cancel.store(true, std::memory_order_release);
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_finished.load(std::memory_order_acquire))
            m_snapshot.cancelling = true;
    }

    ProgressSnapshot ExportProgress::Snapshot() const {
        ProgressSnapshot result;
        std::chrono::steady_clock::time_point end;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            result = m_snapshot;
            end = m_endSet ? m_end : std::chrono::steady_clock::now();
        }

        result.elapsedSeconds = std::chrono::duration<double>(end - m_start).count();
        return result;
    }
}
