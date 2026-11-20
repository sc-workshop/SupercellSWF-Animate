#pragma once

#include "Progress/ExportProgress.h"

#include <memory>

namespace sc::Adobe {
    constexpr float PROGRESS_WINDOW_WIDTH = 820.0f;
    constexpr float PROGRESS_WINDOW_HEIGHT = 440.0f;

    /// @brief Platform window/renderer pair for the ImGui progress dialog.
    class IProgressBackend {
    public:
        virtual ~IProgressBackend() = default;

        virtual bool Create(const char* title, ExportProgress* progress) = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual bool IsOpen() const = 0;
        virtual void Destroy() = 0;
    };

    std::unique_ptr<IProgressBackend> CreateProgressBackend();
}
