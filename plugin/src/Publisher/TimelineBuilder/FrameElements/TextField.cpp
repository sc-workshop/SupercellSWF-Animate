#include "Publisher/TimelineBuilder/FrameElements/TextField.h"

#define isEqual(a,b) (fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * 0.01))

namespace sc {
	namespace Adobe {
		bool TextFieldInfo::operator==(TextFieldInfo const& other) const {
			return
				text == other.text &&

				isEqual(bound.bottomRight.x, bound.bottomRight.x) &&
				isEqual(bound.bottomRight.y, bound.bottomRight.y) &&
				isEqual(bound.topLeft.x, bound.topLeft.x) &&
				isEqual(bound.topLeft.y, bound.topLeft.y) &&

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
				;
		}
	}
}