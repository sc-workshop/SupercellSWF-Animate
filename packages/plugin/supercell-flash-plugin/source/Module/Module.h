#pragma once

#include "AnimateCore.h"
#include "AnimateModule.h"
#include "AnimatePublisher.h"
#include "Module/FCMClasses.h"
#include "Module/Localization.h"
#include "Module/Version.h"
#include "Plugin/FeatureMatrix.h"
#include "Plugin/Publisher.h"
#include "Progress/ExportProgress.h"

#pragma warning(push, 0)
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/spdlog.h"

#include <fmt/xchar.h>
#pragma warning(pop)

#include "core/memory/ref.h"

namespace sc::Adobe {
    class SCPlugin : public FCM::PluginModule {
    public:
        static const Animate::ModuleInfo SCPluginInfo;

    public:
        using Publisher = Animate::Publisher::GenericPublisher<SCConfig, SCPublisher>;
        using FeatureMatrixLoader = SCFeatureMatrixLoader;
        using DocType = Animate::DocType::GenericDocumentType<Publisher, FeatureMatrix, FeatureMatrixLoader>;

    public:
        static SCPlugin& Instance() { return (SCPlugin&) FCM::PluginModule::Instance(); }

    public:
        SCPlugin(FCM::PIFCMCallback callback);
        NON_COPYABLE(SCPlugin)

    public:
        void InitializeLogger();
        void InitializeLogData();

        ExportProgress* Progress() const { return m_progress; }
        void SetProgress(ExportProgress* progress) { m_progress = progress; }

    public:
        Localization locale;

        // Plugin Logger
        wk::Ref<spdlog::logger> logger;

        // Log File for logger
        std::ofstream logger_file;

    private:
        // Active publish state, set for the lifetime of Publish()
        // Worker threads use it for progress and cancellation
        // The publish thread owns the progress window
        ExportProgress* m_progress = nullptr;
    };
}
