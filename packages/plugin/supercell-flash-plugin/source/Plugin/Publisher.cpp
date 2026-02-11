#include "Publisher.h"

#include <shared_mutex>

#include "Module/Module.h"
#include "Writer/Writer.h"
#include "Module/SCPluginException.h"
#include "wx/init.h"

using namespace Animate::Publisher;
using namespace sc::Adobe;

template<>
SCConfig&
::Animate::Publisher::GenericPublisher<SCConfig, SCPublisher>::ActiveConfig()
{
	static SCConfig config;
	return config;
}

template<>
const Animate::FCMPluginID&
::Animate::Publisher::GenericPublisher<SCConfig, SCPublisher>::PluginID()
{
    static Animate::FCMPluginID id(
        CLSID_Publisher,
        CLSID_DocType,
        CLSID_FeatureMatrix
    );
    return id;
}

namespace sc {
	namespace Adobe {
    
		void SCPublisher::PublishDocuments()
		{
			SCPlugin& context = SCPlugin::Instance();
			SCConfig& config = SCPlugin::Publisher::ActiveConfig();

			StatusComponent* publishStatus = context.Window()->CreateStatusBarComponent(
				context.locale.GetString("TID_STATUS_INIT")
			);

			SCWriter writer;
			ResourcePublisher publisher(writer);

			{
				if (config.exportToExternal)
				{
					if (fs::exists(config.exportToExternalPath))
					{
						publishStatus->SetStatusLabel(context.locale.GetString("TID_EXTERNAL_FILE_LOAD"));
						uint16_t offset = writer.LoadExternal(config.exportToExternalPath);
						publisher.SetIdOffset(offset);
					}
					else
					{
						throw SCPluginException("TID_SWF_MISSING_EXTERNAL_FILE", config.exportToExternalPath.wstring().c_str());
					}
				}
			}

			auto publish = [&](Animate::DOM::PIFLADocument document) {
				fs::path document_path = context.falloc->GetString16(
					document,
					&Animate::DOM::IFLADocument::GetPath
				);

				publishStatus->SetStatusLabel(
					context.locale.GetString(
						"TID_WINDOW_TITLE",
						document_path.filename().u16string().c_str()
					)
				);

				config.activeDocument = document;
				publisher.PublishDocument(document);
			};

			publish(config.activeDocument);
			for (auto& document : m_loaded_documents) {
				publish(document);

				// Close external document after publishing (may kinda slow but leave as it is for now)
				CloseDocument(document);

				// And mark as used
				document = nullptr;
			}

			publishStatus->SetStatus(
				context.locale.GetString("TID_STATUS_SAVE")
			);

			publisher.Finalize();
		}

		SCPublisher::~SCPublisher()
		{
			// Close remaining documents in case of errors or smth, to avoid document blocking
			for (auto document : m_loaded_documents) {
				if (!document)
					continue;

				CloseDocument(document);
			}
		}

		bool SCPublisher::VerifyDocument(const fs::path& path)
		{
			if (!fs::exists(path))
				return false;

			return true;
		}

		void SCPublisher::LoadDocument(const fs::path& path, Animate::DOM::PIFLADocument& document)
		{
			using Animate::DOM::Service::Document::IFLADocService;
			using namespace Animate::DOM;

			SCPlugin& context = SCPlugin::Instance();
			const auto& service = context.GetService<IFLADocService>(FLA_DOC_SERVICE);

			FCM::Result status = service->OpenDocument(
				(FCM::CStringRep16)FCM::Locale::ToUtf16(path.string()).c_str(),
				document
			);

			if (!FCM_SUCCESS_CODE(status) || !document) {
				throw SCPluginException("TID_LOAD_EXTERNAL_DOCUMENT_FAILED", path.c_str());
			}

			FCM::FCMGUID docId;
			status = document->GetTypeId(docId);

			if (!FCM_SUCCESS_CODE(status) || docId != CLSID_DocType) {
				service->CloseDocument(document);
				throw SCPluginException("TID_EXTERNAL_DOCUMENT_WRONG_TYPE", path.c_str());
			}
		}

		void SCPublisher::CloseDocument(Animate::DOM::PIFLADocument& document)
		{
			using Animate::DOM::Service::Document::IFLADocService;
			using namespace Animate::DOM;

			SCPlugin& context = SCPlugin::Instance();
			const auto& service = context.GetService<IFLADocService>(FLA_DOC_SERVICE);
			service->CloseDocument(document);
		}

		void SCPublisher::Publish(const SCConfig& /*config*/)
		{
			SCPlugin& context = SCPlugin::Instance();

			context.logger->info("--------------------------- Called Publish -----------------------------");

			auto start = std::chrono::high_resolution_clock::now();

			std::shared_mutex publishing_ui;
			// Must be unlocked when ui is ready to use
			publishing_ui.lock();

			context.logger->info("Starting UI...");
			std::thread progressWindow(
				[&context, &publishing_ui]()
				{
                    int argc = 0;
                    char** argv = nullptr;
                    
					context.InitializeWindow();
					bool entry_start_status = wxEntryStart(argc, argv);
					
					if (!entry_start_status)
					{
						auto message = wxSysErrorMsgStr(0);
						context.logger->error("wxSysErrorMsgStr: {}", message.ToStdString());
					}

					bool init_status = wxTheApp->CallOnInit();
					context.logger->info("Windows init finished with status: {}", init_status);
					if (!init_status)
					{
						auto message = wxSysErrorMsgStr(0);
						context.logger->error("wxSysErrorMsgStr: {}", message.ToStdString());
					}

					context.logger->info("UI successfully started");
					publishing_ui.unlock();

					wxTheApp->OnRun();
					wxEntryCleanup();
				}
			);

			FCM::Result result = FCM_SUCCESS;
			// Block thread until publishing ui is ready
			publishing_ui.lock();
			context.logger->info("Starting publishing...");

			try {
				DoPublish();
			}
			catch (const FCM::FCMPluginException& exception)
			{
				std::string reason;
				{
					std::stringstream message;
					auto& symbol = exception.Symbol();
					if (!symbol.name.empty())
					{
						message << " [" << FCM::Locale::ToUtf8(symbol.name) << "] ";
					}
					message << exception.what();
					reason = message.str();
				}

				context.Window()->ThrowException(reason);
				context.Window()->readyToExit = true;
				result = FCM_EXPORT_FAILED;
			}
			catch (const std::exception& exception) {
				context.Window()->ThrowException(exception.what());
				context.Window()->readyToExit = true;
				result = FCM_EXPORT_FAILED;
			}
			catch (...) {
				context.logger->error("Publishing finished with unknown exception!");

				context.Window()->readyToExit = true;
				result = FCM_EXPORT_FAILED;
			}

			publishing_ui.unlock();
			progressWindow.join();
			context.logger->info("Publisher finished with status: {}", (uint32_t)result);

			auto end = std::chrono::high_resolution_clock::now();

			long long int executionTime = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
			context.logger->info("Execution time: {}s", executionTime);
			context.logger->info("------------------------------------------------------------------------");

			context.Trace(
				context.locale.GetString("TID_EXPORT_TIME_STATUS", executionTime)
			);
		}

		void SCPublisher::DoPublish()
		{
			SCPlugin& context = SCPlugin::Instance();
			const SCConfig& config = SCPlugin::Publisher::ActiveConfig();

			if (config.useMultiDocument) {
				StatusComponent* documentProgress = context.Window()->CreateStatusBarComponent(
					context.locale.GetString("TID_LABEL_DOCUMENT_LOADING")
				);

				for (const auto& path : config.documentsPaths) {
					if (!VerifyDocument(path))
						throw SCPluginException("TID_INVALID_EXTERNAL_DOCUMENT", path.c_str());
				}

				for (auto& path : config.documentsPaths) {
					LoadDocument(path, m_loaded_documents.emplace_back());

					documentProgress->SetStatus(fs::path(path).filename().u16string().c_str());
				}

				context.Window()->DestroyStatusBar(documentProgress);
			}

			PublishDocuments();
			context.DestroyWindow();
		}
	}
}
