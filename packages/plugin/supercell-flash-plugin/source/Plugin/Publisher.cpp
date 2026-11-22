#include "Publisher.h"

#include "Cache/CacheManager.h"
#include "Module/Module.h"
#include "Module/SCPluginException.h"
#include "Progress/ProgressUI.h"
#include "Writer/Writer.h"

#include <fmt/format.h>
#include <shared_mutex>
#include <sstream>

using namespace Animate::Publisher;
using namespace sc::Adobe;

namespace {
    std::string DocumentName(const FCM::AutoPtr<Animate::DOM::IFLADocument>& document) {
        SCPlugin& context = SCPlugin::Instance();
        fs::path path;
        {
            FCM::PluginModule::HostGate document_gate(FCM::PluginModule::HostScope::Library);
            path = context.falloc->GetString16(document.m_Ptr, &Animate::DOM::IFLADocument::GetPath);
        }
        return path.filename().string();
    }
}

template <>
SCConfig& ::Animate::Publisher::GenericPublisher<SCConfig, SCPublisher>::ActiveConfig() {
    static SCConfig config;
    return config;
}

template <>
const Animate::FCMPluginID& ::Animate::Publisher::GenericPublisher<SCConfig, SCPublisher>::PluginID() {
    static Animate::FCMPluginID id(CLSID_Publisher, CLSID_DocType, CLSID_FeatureMatrix);
    return id;
}

namespace sc::Adobe {
    void SCPublisher::PublishDocuments(const std::shared_ptr<Cache::CacheManager>& cache) {
        SCPlugin& context = SCPlugin::Instance();
        SCConfig& config = SCPlugin::Publisher::ActiveConfig();
        ExportProgress* progress = context.Progress();
        using Clock = std::chrono::steady_clock;

        const auto functionStart = Clock::now();
        context.logger->info("[Perf] PublishDocuments enter");

        // Loading documents
        std::vector<Animate::DOM::PIFLADocument> pending;
        pending.push_back(config.activeDocument);
        for (auto document : m_loadedDocuments) {
            if (document) {
                pending.push_back(document);
            }
        }

        progress->SetDocuments(0, pending.size(), DocumentName(config.activeDocument));

        // Creating root writer objects
        SCWriter writer(cache);
        ResourcePublisher publisher(writer);
        publisher.diagnostics.SetSink(
            [&context](const std::string& message) { context.logger->info("[SDK] {}", message); });

        Clock::time_point prepareStart {};
        Clock::time_point commitStart {};
        Clock::time_point lastDiag {};
        bool havePrepare = false;
        bool haveCommit = false;

        publisher.cancellationPoint = [progress] { ThrowIfCancelled(progress); };
        publisher.prepareProgress = [&, progress](size_t done, size_t total) {
            const auto now = Clock::now();
            if (done == 0) {
                prepareStart = now;
                lastDiag = now;
                havePrepare = true;
                haveCommit = false;
            }
            if (!haveCommit && now - lastDiag >= std::chrono::seconds(10)) {
                lastDiag = now;
                context.logger->info("[Perf] prepared={}/{} cache={}{}",
                                     done,
                                     total,
                                     cache ? cache->Summary() : std::string("<none>"),
                                     publisher.diagnostics.Summary());
            }
            progress->SetCounts(done, total);
        };
        publisher.commitStarted = [&, progress](size_t total) {
            commitStart = Clock::now();
            haveCommit = true;
            progress->BeginPhase(ExportPhase::Committing, total, context.locale.GetU8String("TID_PHASE_COMMITTING"));
        };

        // Handling external base file
        if (config.exportToExternal) {
            progress->BeginPhase(ExportPhase::ExternalFile, 0, context.locale.GetU8String("TID_EXTERNAL_FILE_LOAD"));

            if (fs::exists(config.exportToExternalPath)) {
                uint16_t offset = writer.LoadExternal(config.exportToExternalPath);
                publisher.SetIdOffset(offset);
            } else {
                throw SCPluginException("TID_SWF_MISSING_EXTERNAL_FILE", config.exportToExternalPath.string());
            }
        }

        ThrowIfCancelled(progress);

        size_t documentIndex = 0;
        auto publish = [&](Animate::DOM::PIFLADocument document, const fs::path& documentPath) {
            ThrowIfCancelled(progress);

            const auto documentStart = Clock::now();
            context.logger->info("[Perf] document-start={} path={}", documentIndex, documentPath.string());

            // Setting up active document
            std::string name = DocumentName(document);
            progress->SetDocuments(documentIndex + 1, pending.size(), name);
            progress->BeginPhase(ExportPhase::Extracting, 0, context.locale.GetU8String("TID_WINDOW_TITLE", name));
            writer.SetCacheDocument(documentPath);
            config.activeDocument = document;

            const auto publishStart = Clock::now();
            context.logger->info("[Perf] PublishDocument enter");
            publisher.PublishDocument(document);
            context.logger
                ->info("[Perf] PublishDocument exit={}ms",
                       std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - publishStart).count());

            if (cache) {
                const auto rootKeys = publisher.RootCacheKeys();
                bool complete = true;
                for (const auto& key : rootKeys) {
                    if (key == Animate::Publisher::SymbolCacheKey {}) {
                        complete = false;
                        break;
                    }
                }
                cache->SetRootKeys(rootKeys, complete);
            }

            if (havePrepare) {
                const auto end = Clock::now();
                const auto prepareMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                           (haveCommit ? commitStart : end) - prepareStart)
                                           .count();
                const auto commitMs =
                    haveCommit ? std::chrono::duration_cast<std::chrono::milliseconds>(end - commitStart).count() : 0;
                context.logger->info("[Timing] document={} prepare={}ms commit={}ms{}",
                                     name,
                                     prepareMs,
                                     commitMs,
                                     publisher.diagnostics.Summary());
            }

            documentIndex++;
        };

        publish(config.activeDocument, config.documentPath);
        for (size_t index = 0; index < m_loadedDocuments.size(); ++index) {
            auto& document = m_loadedDocuments[index];
            if (document) {
                fs::path documentPath;
                if (index < config.documentsPaths.size()) {
                    documentPath = config.documentsPaths[index];
                }

                publish(document, documentPath);

                // Close external document after publishing
                CloseDocument(document);

                // And mark as used
                document = nullptr;
                continue;
            }

            if (cache && index < m_cachedDocuments.size() && !m_cachedDocuments[index].fileGuid.empty()) {
                const auto& cached = m_cachedDocuments[index];
                const auto replayStart = Clock::now();
                context.logger->info("[Perf] replay-cached-document={} roots={}", index, cached.roots.size());
                cache->BeginReplay(cached.fileGuid);
                const bool replayed = publisher.PublishCachedRoots(cached.roots);
                cache->EndReplay();
                context.logger
                    ->info("[Perf] replay-cached-document-done={}ms ok={}",
                           std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - replayStart).count(),
                           replayed ? 1 : 0);
                if (!replayed) {
                    // Not every symbol is cacheable (e.g. custom properties). A cached document
                    // can therefore be incomplete; load and publish the FLA normally rather
                    // than failing the whole export.
                    const fs::path& cachedPath = config.documentsPaths[index];
                    context.logger->info("[Cache] cached replay incomplete, loading '{}'", cachedPath.string());
                    LoadDocument(cachedPath, document);
                    if (document) {
                        publish(document, cachedPath);
                        CloseDocument(document);
                        document = nullptr;
                    }
                }
            }
        }

        progress->SetDetail(context.locale.GetU8String("TID_STATUS_SAVE"));

        // Finalize the writer: builds atlases and writes the .sc (plus external textures).
        // This was accidentally dropped in the ImGui/parallel refactor; without it publish
        // reports success but never produces an output file.
        publisher.Finalize();

        context.logger
            ->info("[Perf] PublishDocuments exit={}ms",
                   std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - functionStart).count());
    }

    SCPublisher::~SCPublisher() {
        // Close remaining documents in case of errors or smth, to avoid document blocking
        for (auto document : m_loadedDocuments) {
            if (!document)
                continue;

            CloseDocument(document);
        }
    }

    bool SCPublisher::VerifyDocument(const fs::path& path) {
        if (!fs::exists(path))
            return false;

        return true;
    }

    void SCPublisher::LoadDocument(const fs::path& path, Animate::DOM::PIFLADocument& document) {
        using Animate::DOM::Service::Document::IFLADocService;
        using namespace Animate::DOM;

        SCPlugin& context = SCPlugin::Instance();
        const auto& service = context.GetService<IFLADocService>(FLA_DOC_SERVICE);

        FCM::Result status;
        {
            FCM::PluginModule::HostGate document_gate(FCM::PluginModule::HostScope::Library);
            status = service->OpenDocument((FCM::CStringRep16) FCM::Locale::ToUtf16(path.string()).c_str(), document);
        }

        if (!FCM_SUCCESS_CODE(status) || !document) {
            throw SCPluginException("TID_LOAD_EXTERNAL_DOCUMENT_FAILED", path.string());
        }

        FCM::FCMGUID docId;
        {
            FCM::PluginModule::HostGate document_gate(FCM::PluginModule::HostScope::Library);
            status = document->GetTypeId(docId);
        }

        if (!FCM_SUCCESS_CODE(status) || docId != CLSID_DocType) {
            FCM::PluginModule::HostGate document_gate(FCM::PluginModule::HostScope::Library);
            service->CloseDocument(document);
            throw SCPluginException("TID_EXTERNAL_DOCUMENT_WRONG_TYPE", path.string());
        }
    }

    void SCPublisher::CloseDocument(Animate::DOM::PIFLADocument& document) {
        using Animate::DOM::Service::Document::IFLADocService;
        using namespace Animate::DOM;

        SCPlugin& context = SCPlugin::Instance();
        const auto& service = context.GetService<IFLADocService>(FLA_DOC_SERVICE);
        FCM::PluginModule::HostGate document_gate(FCM::PluginModule::HostScope::Library);
        service->CloseDocument(document);
    }

    FCM::Result SCPublisher::StartPublishing(ExportProgress& progress,
                                             const std::shared_ptr<Cache::CacheManager>& cache) {
        SCPlugin& context = SCPlugin::Instance();

        try {
            DoPublish(cache);
        }
        // User cancelled exceptuon
        catch (const ExportCancelled&) {
            context.logger->info("Publishing cancelled by user");
            progress.Finish(ExportOutcome::Cancelled);
            return FCM_OPERATION_CANCELLED;
        }
        // FCM Interface exception
        catch (const FCM::FCMPluginException& exception) {
            std::ostringstream message;
            const auto& symbol = exception.Symbol();
            if (!symbol.name.empty())
                message << "[" << FCM::Locale::ToUtf8(symbol.name) << "] ";
            message << exception.what();

            std::string reason = message.str();
            context.logger->error("Publishing failed: {}", reason);
            progress.Finish(ExportOutcome::Failure, reason);
            return FCM_EXPORT_FAILED;
        }
        // Probably our own code exception
        catch (const std::exception& exception) {
            context.logger->error("Publishing failed: {}", exception.what());
            progress.Finish(ExportOutcome::Failure, exception.what());
            return FCM_EXPORT_FAILED;
        }
        // Rest of exceptions
        catch (...) {
            context.logger->error("Publishing finished with unknown exception!");
            progress.Finish(ExportOutcome::Failure, context.locale.GetU8String("TID_UNKNOWN_EXCEPTION"));
            return FCM_EXPORT_FAILED;
        }

        cache->Commit();
        progress.Finish(ExportOutcome::Success);
        return FCM_SUCCESS;
    }

    void SCPublisher::Publish(const SCConfig& /*config*/) {
        SCPlugin& context = SCPlugin::Instance();

        context.logger->info("--------------------------- Called Publish -----------------------------");
        auto start = std::chrono::high_resolution_clock::now();

        ExportProgress progress;
        progress.Reset();
        context.SetProgress(&progress);

        auto cache = std::make_shared<Cache::CacheManager>(SCPlugin::Publisher::ActiveConfig());

        FCM::Result result = FCM_SUCCESS;
        context.logger->info("UI successfully inited. Starting publishing...");
        std::thread worker([&]() { result = StartPublishing(progress, cache); });

        try {
            ProgressUIConfig uiConfig;
            uiConfig.progress = &progress;
            uiConfig.locale = &context.locale;
            if (!RunProgressUI(uiConfig))
                context.logger->warn("Progress window could not be created; publishing continues in background");
        } catch (const std::exception& exception) {
            context.logger->error("Progress UI failed: {}", exception.what());
            progress.RequestCancel();
        }

        worker.join();
        context.SetProgress(nullptr);

        if (result != FCM_SUCCESS) {
            std::string error = progress.Snapshot().error;
            if (!error.empty())
                context.Trace("{}: {}", context.locale.GetU8String("TID_PHASE_FAILED"), error);
        }

        context.logger->info("Publisher finished with status: {}", (uint32_t) result);
        if (result == FCM_SUCCESS) {
            // Full technical summary (fileGUID, policy folder, timings) stays in the text log.
            context.logger->info("[Cache] {}", cache->Summary());
            context.Trace(cache->UserSummary());
        }
        auto end = std::chrono::high_resolution_clock::now();
        long long int executionTime = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
        context.logger->info("Execution time: {}s", executionTime);
        context.logger->info("------------------------------------------------------------------------");

        context.Trace(context.locale.GetString("TID_EXPORT_TIME_STATUS", executionTime));
    }

    void SCPublisher::DoPublish(const std::shared_ptr<Cache::CacheManager>& cache) {
        SCPlugin& context = SCPlugin::Instance();
        const SCConfig& config = SCPlugin::Publisher::ActiveConfig();
        ExportProgress* progress = context.Progress();
        context.logger->info("[Perf] DoPublish enter externalDocs={}", config.documentsPaths.size());

        // Close external documents on every exit path, before StartPublishing reports the
        // outcome. Otherwise the host stays busy in this destructor after the dialog is gone.
        struct LoadedDocumentsGuard {
            SCPublisher& owner;

            ~LoadedDocumentsGuard() {
                for (auto& document : owner.m_loadedDocuments) {
                    if (document) {
                        SCPublisher::CloseDocument(document);
                        document = nullptr;
                    }
                }
                owner.m_loadedDocuments.clear();
                owner.m_cachedDocuments.clear();
            }
        } documentsGuard {*this};

        // Validate external input before opening any document. Otherwise a bad path wastes the
        // whole load pass and the error only appears after a long, host-heavy cleanup.
        if (config.exportToExternal && !fs::exists(config.exportToExternalPath))
            throw SCPluginException("TID_SWF_MISSING_EXTERNAL_FILE", config.exportToExternalPath.string());

        m_loadedDocuments.clear();
        m_cachedDocuments.clear();

        if (config.useMultiDocument) {
            const size_t documentCount = config.documentsPaths.size();
            progress->BeginPhase(ExportPhase::LoadingDocuments,
                                 documentCount,
                                 context.locale.GetU8String("TID_LABEL_DOCUMENT_LOADING"));

            for (const auto& path : config.documentsPaths) {
                if (!VerifyDocument(path))
                    throw SCPluginException("TID_INVALID_EXTERNAL_DOCUMENT", path.string());
            }

            m_loadedDocuments.resize(documentCount, nullptr);
            m_cachedDocuments.resize(documentCount);

            size_t index = 0;
            for (const auto& path : config.documentsPaths) {
                ThrowIfCancelled(progress);

                Cache::CacheManager::CachedDocument cached;
                const auto externalStart = std::chrono::high_resolution_clock::now();
                bool cachedSkipped = false;
                if (cache && cache->TryGetCachedDocument(path, cached)) {
                    // Raw .fla hash and cached export roots match: Animate must not load this FLA
                    m_cachedDocuments[index] = std::move(cached);
                    cachedSkipped = true;
                    context.logger->info("[Cache] skip external load '{}': cached roots={}",
                                         path.string(),
                                         m_cachedDocuments[index].roots.size());
                } else {
                    LoadDocument(path, m_loadedDocuments[index]);
                }
                context.logger->info("[Perf] external-doc={} cached={} ms={} path={}",
                                     index,
                                     cachedSkipped ? 1 : 0,
                                     std::chrono::duration_cast<std::chrono::milliseconds>(
                                         std::chrono::high_resolution_clock::now() - externalStart)
                                         .count(),
                                     path.string());

                std::string name = path.filename().string();
                progress->SetDocuments(++index, documentCount, name);
                progress->SetDetail(name);
            }
        }

        ThrowIfCancelled(progress);
        PublishDocuments(cache);
    }
}
