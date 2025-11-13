#pragma once

#include <chrono>
#include <thread>

#include "AnimateModule.h"
#include "Module/Config.h"
#include "Animate/app/DOM/Service/Document/IFLADocService.h"

namespace sc {
	namespace Adobe {
		class SCPublisher : public Animate::Publisher::GenericPublisherInterface<SCConfig>
		{
		public:
			static bool VerifyDocument(const std::string& path);
			static void LoadDocument(const fs::path& path, Animate::DOM::PIFLADocument& document);
			static void CloseDocument(Animate::DOM::PIFLADocument& document);

		public:
			virtual void Publish(const SCConfig& config);
			
			void PublishDocument(Animate::DOM::PIFLADocument document);
			void DoPublish();
		};
	}
}