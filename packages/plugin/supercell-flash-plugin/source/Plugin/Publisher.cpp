#include "Publisher.h"

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
    void SCPublisher::PublishDocuments() {
        SCPlugin& context = SCPlugin::Instance();
        SCConfig& config = SCPlugin::Publisher::ActiveConfig();
        ExportProgress* progress = context.Progress();

        // Loading documents
        std::vector<Animate::DOM::PIFLADocument> pending;
        pending.push_back(config.activeDocument);
        for (auto document : m_loaded_documents) {
            if (document) {
                pending.push_back(document);
            }
        }

        progress->SetDocuments(0, pending.size(), DocumentName(config.activeDocument));

        // Creating root writer objects
        SCWriter writer;
        ResourcePublisher publisher(writer);
        publisher.cancellationPoint = [progress] { ThrowIfCancelled(progress); };
        publisher.prepareProgress = [progress](size_t done, size_t total) { progress->SetCounts(done, total); };
        publisher.commitStarted = [progress, &context](size_t total) {
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
        auto publish = [&](Animate::DOM::PIFLADocument document) {
            ThrowIfCancelled(progress);

            std::string name = DocumentName(document);
            progress->SetDocuments(documentIndex + 1, pending.size(), name);
            progress->BeginPhase(ExportPhase::Extracting, 0, context.locale.GetU8String("TID_WINDOW_TITLE", name));

            config.activeDocument = document;
            publisher.PublishDocument(document);
            documentIndex++;
        };

        publish(config.activeDocument);
        for (auto& document : m_loaded_documents) {
            if (!document)
                continue;

            publish(document);

            // Close external document after publishing
            CloseDocument(document);

            // And mark as used
            document = nullptr;
        }

        progress->SetDetail(context.locale.GetU8String("TID_STATUS_SAVE"));
    }

    SCPublisher::~SCPublisher() {
        // Close remaining documents in case of errors or smth, to avoid document blocking
        for (auto document : m_loaded_documents) {
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

    FCM::Result SCPublisher::StartPublishing(ExportProgress& progress) {
        SCPlugin& context = SCPlugin::Instance();

        try {
            DoPublish();
        } catch (const ExportCancelled&) {
            context.logger->info("Publishing cancelled by user");
            progress.Finish(ExportOutcome::Cancelled);
            return FCM_OPERATION_CANCELLED;
        } catch (const FCM::FCMPluginException& exception) {
            std::ostringstream message;
            const auto& symbol = exception.Symbol();
            if (!symbol.name.empty())
                message << "[" << FCM::Locale::ToUtf8(symbol.name) << "] ";
            message << exception.what();

            std::string reason = message.str();
            context.logger->error("Publishing failed: {}", reason);
            progress.Finish(ExportOutcome::Failure, reason);
            return FCM_EXPORT_FAILED;
        } catch (const std::exception& exception) {
            context.logger->error("Publishing failed: {}", exception.what());
            progress.Finish(ExportOutcome::Failure, exception.what());
            return FCM_EXPORT_FAILED;
        } catch (...) {
            context.logger->error("Publishing finished with unknown exception!");
            progress.Finish(ExportOutcome::Failure, context.locale.GetU8String("TID_UNKNOWN_EXCEPTION"));
            return FCM_EXPORT_FAILED;
        }

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

        FCM::Result result = FCM_SUCCESS;
        context.logger->info("UI successfully inited. Starting publishing...");
        std::thread worker([&]() { result = StartPublishing(progress); });

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
        auto end = std::chrono::high_resolution_clock::now();
        long long int executionTime = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
        context.logger->info("Execution time: {}s", executionTime);
        context.logger->info("------------------------------------------------------------------------");

        context.Trace(context.locale.GetString("TID_EXPORT_TIME_STATUS", executionTime));
    }

    void SCPublisher::DoPublish() {
        SCPlugin& context = SCPlugin::Instance();
        const SCConfig& config = SCPlugin::Publisher::ActiveConfig();
        ExportProgress* progress = context.Progress();

        // Close external documents on every exit path, before StartPublishing reports the
        // outcome. Otherwise the host stays busy in this destructor after the dialog is gone.
        struct LoadedDocumentsGuard {
            SCPublisher& owner;

            ~LoadedDocumentsGuard() {
                for (auto& document : owner.m_loaded_documents) {
                    if (document) {
                        SCPublisher::CloseDocument(document);
                        document = nullptr;
                    }
                }
                owner.m_loaded_documents.clear();
            }
        } documentsGuard {*this};

        // Validate external input before opening any document. Otherwise a bad path wastes the
        // whole load pass and the error only appears after a long, host-heavy cleanup.
        if (config.exportToExternal && !fs::exists(config.exportToExternalPath))
            throw SCPluginException("TID_SWF_MISSING_EXTERNAL_FILE", config.exportToExternalPath.string());

        if (config.useMultiDocument) {
            const size_t documentCount = config.documentsPaths.size();
            progress->BeginPhase(ExportPhase::LoadingDocuments,
                                 documentCount,
                                 context.locale.GetU8String("TID_LABEL_DOCUMENT_LOADING"));

            for (const auto& path : config.documentsPaths) {
                if (!VerifyDocument(path))
                    throw SCPluginException("TID_INVALID_EXTERNAL_DOCUMENT", path.string());
            }

            size_t index = 0;
            for (auto& path : config.documentsPaths) {
                ThrowIfCancelled(progress);

                LoadDocument(path, m_loaded_documents.emplace_back());

                std::string name = path.filename().string();
                progress->SetDocuments(++index, documentCount, name);
                progress->SetDetail(name);
            }
        }

        ThrowIfCancelled(progress);
        PublishDocuments();
    }
}
