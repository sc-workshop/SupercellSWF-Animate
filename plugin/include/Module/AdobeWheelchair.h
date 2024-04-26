#pragma once

#include "AnimateSDK/core/common/FCMTypes.h"

namespace sc
{
	namespace Adobe
	{

		// Class with any kind of offset to different classes
		// This class was created with the goal of fixing some Adobe Wheelchairs by obtaining the necessary data directly, using offsets in memory
		// not good, but its the only way...
		class AdobeWheelchair
		{
		public:
			static AdobeWheelchair& Instance()
			{
				static AdobeWheelchair singleton;
				return singleton;
			}

		public:
			AdobeWheelchair() {};
			~AdobeWheelchair() = default;
			AdobeWheelchair(const AdobeWheelchair&) = delete;
			AdobeWheelchair& operator=(const AdobeWheelchair&) = delete;

		public:
			// Sets offsets by Animate version
			FCM::Result RunWheelchair(FCM::U_Int32 version);

		public:
			// For some reason SDK does not support 9slice...
			size_t CPicPage_Is9SliceEnabled = 0;
			size_t CPicPage_9SliceGuides = 0;
		};
	}
}