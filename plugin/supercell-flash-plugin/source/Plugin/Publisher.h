#pragma once

#include <chrono>
#include <thread>

#include "AnimateModule.h"
#include "Module/Config.h"

namespace sc {
	namespace Adobe {
		class SCPublisher : public Animate::Publisher::GenericPublisherInterface<SCConfig>
		{
		public:
			virtual void Publish(const SCConfig& config);

			void DoPublish();
		};
	}
}