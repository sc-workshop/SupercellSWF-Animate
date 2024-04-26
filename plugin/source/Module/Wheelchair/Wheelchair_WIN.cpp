#include "Module/AdobeWheelchair.h"

#include "AnimateSDK/core/common/FCMErrors.h"

namespace sc
{
	namespace Adobe
	{
		FCM::Result AdobeWheelchair::RunWheelchair(FCM::U_Int32 version)
		{
			size_t major = ((version >> 24) & 0xFF);

			// Animate 2024
			if (major == 24)
			{
				CPicPage_Is9SliceEnabled = 436;
				CPicPage_9SliceGuides = 1748;

				return FCM_SUCCESS;
			}

			// Animate 2023 and lower
			if (major <= 23)
			{
				CPicPage_Is9SliceEnabled = 1624;
				CPicPage_9SliceGuides = 1628;

				return FCM_SUCCESS;
			}

			return FCM_PLUGIN_LOAD_FAILED;
		}
	}
}