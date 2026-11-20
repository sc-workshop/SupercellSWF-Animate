#pragma once

#include "Progress/ExportProgress.h"

#include <string>

namespace sc::Adobe {
    class Localization;

    struct ProgressUIConfig {
        ExportProgress* progress = nullptr;
        Localization* locale = nullptr;
    };

    /// @brief Runs the modal progress window on the calling (publish) thread.
    /// @return True when the window was created and shown.
    bool RunProgressUI(const ProgressUIConfig& config);
}
