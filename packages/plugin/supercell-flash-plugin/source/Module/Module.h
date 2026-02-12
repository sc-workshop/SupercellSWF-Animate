#pragma once

#include "AnimateCore.h"
#include "AnimateModule.h"
#include "AnimatePublisher.h"

#include "Module/Localization.h"
#include "Module/FCMClasses.h"
#include "Plugin/Publisher.h"
#include "Plugin/FeatureMatrix.h"
#include "Module/Version.h"
#include "Window/PluginWindowApp.h"

#include <fmt/xchar.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/ostream_sink.h"

#include "core/memory/ref.h"
#include "core/parallel/bs_thread_pool.hpp"

namespace sc {
	namespace Adobe
	{
		class SCPlugin : public FCM::PluginModule
		{
		public:
			static const Animate::ModuleInfo SCPluginInfo;

		public:
			using Publisher = Animate::Publisher::GenericPublisher<SCConfig, SCPublisher>;
			using FeatureMatrixLoader = SCFeatureMatrixLoader;
			using DocType = Animate::DocType::GenericDocumentType<Publisher, FeatureMatrix, FeatureMatrixLoader>;

		public:
			static SCPlugin& Instance()
			{
				return (SCPlugin&)FCM::PluginModule::Instance();
			}

		public:
			SCPlugin(FCM::PIFCMCallback callback);
			NON_COPYABLE(SCPlugin)

		public:
			void InitializeLogger();
			void InitializeLogData();

			void InitializeWindow();
			void DestroyWindow();
			PluginWindow* Window();

		public:
			Localization locale;

			// Plugin Logger
			wk::Ref<spdlog::logger> logger;

			// Log File for logger
			std::ofstream logger_file;

			// Thread pool for multiple documents export
			BS::thread_pool<0> threads;

		public:
			// Progress Window
			PluginWindowApp* m_app = nullptr;
		};
	}
}
