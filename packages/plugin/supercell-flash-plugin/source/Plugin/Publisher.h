#pragma once

#include "AnimateModule.h"
#include "Cache/CacheManager.h"
#include "Module/Config.h"
#include "Progress/ExportProgress.h"
#include "animate/app/DOM/Service/Document/IFLADocService.h"

#include <chrono>
#include <memory>
#include <thread>

namespace sc::Adobe {
    namespace Cache {
        class CacheManager;
    }

    class SCPublisher : public Animate::Publisher::GenericPublisherInterface<SCConfig> {
    public:
        SCPublisher() = default;
        virtual ~SCPublisher();

    public:
        static bool VerifyDocument(const fs::path& path);
        static void LoadDocument(const fs::path& path, Animate::DOM::PIFLADocument& document);
        static void CloseDocument(Animate::DOM::PIFLADocument& document);

    public:
        FCM::Result StartPublishing(ExportProgress& progress, const std::shared_ptr<Cache::CacheManager>& cache);

        virtual void Publish(const SCConfig& config);

        void PublishDocuments(const std::shared_ptr<Cache::CacheManager>& cache);
        void DoPublish(const std::shared_ptr<Cache::CacheManager>& cache);

    private:
        // Indexed in parallel with SCConfig::documentsPaths. A nullptr document means this
        // external FLA was skipped and m_cachedDocuments holds its cached export roots.
        std::vector<Animate::DOM::PIFLADocument> m_loadedDocuments;
        std::vector<Cache::CacheManager::CachedDocument> m_cachedDocuments;
    };
}
