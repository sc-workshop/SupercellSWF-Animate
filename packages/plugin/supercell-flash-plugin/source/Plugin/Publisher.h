#pragma once

#include <chrono>
#include <thread>

#include "AnimateModule.h"
#include "Module/Config.h"
#include "animate/app/DOM/Service/Document/IFLADocService.h"

namespace sc {
	namespace Adobe {
		class SCPublisher : public Animate::Publisher::GenericPublisherInterface<SCConfig>
		{
		public:
			SCPublisher() = default;
			virtual ~SCPublisher();

		public:
			static bool VerifyDocument(const fs::path& path);
			static void LoadDocument(const fs::path& path, Animate::DOM::PIFLADocument& document);
			static void CloseDocument(Animate::DOM::PIFLADocument& document);

		public:
			virtual void Publish(const SCConfig& config);
			
			void PublishDocuments();
			void DoPublish();

		private:
			std::vector<Animate::DOM::PIFLADocument> m_loaded_documents;
		};
	}
}
