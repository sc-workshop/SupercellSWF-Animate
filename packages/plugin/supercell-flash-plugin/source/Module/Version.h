#pragma once

#include "PluginConfiguration.h"

#include <stdint.h>

namespace sc::Adobe {
    constexpr uint32_t PLUGIN_VERSION =
        ((PLUGIN_VERSION_MAJOR << 24) | (PLUGIN_VERSION_MINOR << 16) | (PLUGIN_VERSION_MAINTENANCE << 8));

    constexpr char PluginVersion[] = "Version: " __SC_ANIMATE_VERSION__ " | "
                                     "Build date: " __DATE__ " " __TIME__;
}
