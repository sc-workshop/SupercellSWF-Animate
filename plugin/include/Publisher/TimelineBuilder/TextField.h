#pragma once 

#include <string>

#include "Utils/DOMTypes.h"

#include <DOM/FrameElement/IClassicText.h>
#include <DOM/FrameElement/IParagraph.h>
#include <DOM/FrameElement/ITextRun.h>
#include <DOM/FrameElement/ITextBehaviour.h>

using namespace DOM::FrameElement;

#include <math.h>

#define isEqual(a,b) (fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * 0.01))

namespace sc {
	namespace Adobe {
		struct TextFieldInfo {
			DOM::Utils::RECT bound;

			// IClassicText

			u16string text;
			AA_MODE_PROP renderingMode;

			// IClassicText -> IParagraph

			PARAGRAPH_STYLE style;

			// IClassicText -> IParagraph -> ITextRun -> ITextStyle

			DOM::Utils::COLOR fontColor = {0xFF, 0xFF, 0xFF, 0xFF };
			uint16_t fontSize;
			string fontStyle;
			u16string fontName;
			FCM::Boolean autoKern;
			DOM::FrameElement::LineMode lineMode;

			// Glow filter

			bool isOutlined = false;
			DOM::Utils::COLOR outlineColor = { 0xFF, 0xFF, 0xFF, 0xFF };

			bool operator==(TextFieldInfo const& other) {
				return
					text == other.text &&

					floatEqual(bound.bottomRight.x, bound.bottomRight.x) &&
					floatEqual(bound.bottomRight.y, bound.bottomRight.y) &&
					floatEqual(bound.topLeft.x, bound.topLeft.x) &&
					floatEqual(bound.topLeft.y, bound.topLeft.y) &&

					renderingMode.aaMode == other.renderingMode.aaMode &&

					style.alignment == other.style.alignment &&
					style.leftMargin == other.style.leftMargin &&
					style.rightMargin == other.style.rightMargin &&

					*((uint32_t*)&fontColor) == *((uint32_t*)&other.fontColor) &&
					fontSize == other.fontSize &&
					fontStyle == other.fontStyle &&
					fontName == other.fontName &&
					autoKern == other.autoKern &&
					lineMode == other.lineMode &&

					isOutlined == other.isOutlined &&
					(other.isOutlined ? *((uint32_t*)&outlineColor) == *((uint32_t*)&other.outlineColor) : true) 
			;}

		};
	}
}