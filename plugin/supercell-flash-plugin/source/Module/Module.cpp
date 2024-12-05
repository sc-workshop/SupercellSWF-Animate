#include "Module.h"

namespace sc::Adobe {
	const Animate::ModuleInfo SCPlugin::SCPluginInfo = Animate::ModuleInfo(
		PLUGIN_VERSION,
		PUBLISHER_NAME,
		PUBLISHER_UNIVERSAL_NAME,
		"",
		PUBLISH_SETTINGS_UI_ID
	);

	SCPlugin::SCPlugin(FCM::PIFCMCallback callback) : FCM::PluginModule(callback, SCPluginInfo)
	{
		ConstructPlugin<Publisher, DocType, FeatureMatrix>();

		InitializeLogger();
		locale.Load(FCM::PluginModule::LanguageCode());

		InitializeLogData();
	}

	void SCPlugin::InitializeLogger()
	{
		const std::string log_name = std::string(DOCTYPE_UNIVERSAL_NAME).append("_export_log.txt");
		const fs::path log_path = fs::temp_directory_path() / log_name;

		if (fs::exists(log_path))
		{
			fs::remove(log_path);
		}

		logger_file = std::ofstream(log_path);
		auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(logger_file, true);

		logger = std::make_shared<spdlog::logger>(DOCTYPE_NAME, ostream_sink);
		logger->set_pattern("[%H:%M:%S] [%l] >> %v");
		spdlog::set_default_logger(logger);
	}

	void SCPlugin::InitializeLogData()
	{
		auto application = GetService<Animate::Application::Service::IApplicationService>(Animate::Application::Service::APP_SERVICE);

		logger->info("Initializing module");
		logger->info("	Plugin info: {}", PluginVersion);

		logger->info("System Info: ");

		{
			FCM::U_Int32 version;
			application->GetVersion(version);
			logger->info("	App: Adobe Animate {}.{}.{}.{}",
				((version >> 24) & 0xFF),
				((version >> 16) & 0xFF),
				((version >> 8) & 0xFF),
				((version) & 0xFF)
			);
		}

		logger->info("	OS: {}", SystemInfo());

		{
			Animate::AdobeWheelchair& wheelchair = Animate::AdobeWheelchair::Instance();
			if (!wheelchair.IsWheelchairMoving())
			{
				logger->error("Wheelchair is standing still :sadness_combat:. Some functions will be disabled and will not work correctly.");
			}
		}
	}

	void SCPlugin::InitializeWindow()
	{
		m_app = new PluginWindowApp();
		wxApp::SetInstance(m_app);
	}

	void SCPlugin::DestroyWindow()
	{
		if (m_app)
		{
			m_app = nullptr;
		}
	}

	PluginWindow* SCPlugin::Window()
	{
		if (m_app)
		{
			return m_app->window;
		}

		return nullptr;
	}
}