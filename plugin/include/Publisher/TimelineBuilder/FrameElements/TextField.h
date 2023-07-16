#pragma once 

#include <string>

#include "Utils/DOMTypes.h"

#include <DOM/FrameElement/IClassicText.h>
#include <DOM/FrameElement/IParagraph.h>
#include <DOM/FrameElement/ITextRun.h>
#include <DOM/FrameElement/ITextBehaviour.h>

using namespace DOM::FrameElement;

#include <math.h>

namespace sc {
	namespace Adobe {
		struct TextFieldInfo {
			bool operator==(TextFieldInfo const& other) const;

			DOM::Utils::RECT bound;

			// IClassicText

			std::u16string text;
			AA_MODE_PROP renderingMode;

			// IClassicText -> IParagraph

			PARAGRAPH_STYLE style;

			// IClassicText -> IParagraph -> ITextRun -> ITextStyle

			DOM::Utils::COLOR fontColor = {0xFF, 0xFF, 0xFF, 0xFF };
			uint16_t fontSize;
			std::string fontStyle;
			std::u16string fontName;
			FCM::Boolean autoKern;
			DOM::FrameElement::LineMode lineMode;

			// Glow filter

			bool isOutlined = false;
			DOM::Utils::COLOR outlineColor = { 0xFF, 0xFF, 0xFF, 0xFF };
		};
	}
}