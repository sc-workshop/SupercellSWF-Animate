#pragma once

#include <FCMTypes.h>
#include <Utils/DOMTypes.h>
#include <FillStyle/IGradientFillStyle.h>
#include <StrokeStyle/ISolidStrokeStyle.h>

#include <map>

#include "libjson/libjson.h"

#include "Writers/Base/OutputWriter.h"

#define IMAGE_FOLDER "images"
#define SOUND_FOLDER "sounds"

namespace SupercellSWF {
	class JSONOutputWriter : public OutputWriter
	{
	public:
		// Marks the begining of the output
		FCM::Result StartOutput(std::string& outputFileName);

		// Marks the end of the output
		FCM::Result EndOutput();

		// Marks the begining of the Document
		FCM::Result StartDocument(
			const DOM::Utils::COLOR& background,
			FCM::U_Int32 stageHeight,
			FCM::U_Int32 stageWidth,
			FCM::U_Int32 fps);

		// Marks the end of the Document
		FCM::Result EndDocument();

		// Marks the start of a timeline
		FCM::Result StartDefineTimeline();

		// Marks the end of a timeline
		FCM::Result EndDefineTimeline(
			FCM::U_Int32 resId,
			FCM::StringRep16 pName,
			TimelineWriter* pTimelineWriter);

		// Marks the start of a shape
		FCM::Result StartDefineShape();

		// Start of fill region definition
		FCM::Result StartDefineFill();

		// Solid fill style definition
		FCM::Result DefineSolidFillStyle(const DOM::Utils::COLOR& color);

		// Bitmap fill style definition
		FCM::Result DefineBitmapFillStyle(
			FCM::Boolean clipped,
			const DOM::Utils::MATRIX2D& matrix,
			FCM::S_Int32 height,
			FCM::S_Int32 width,
			const std::string& libPathName,
			DOM::LibraryItem::PIMediaItem pMediaItem);

		// Start Linear Gradient fill style definition
		FCM::Result StartDefineLinearGradientFillStyle(
			DOM::FillStyle::GradientSpread spread,
			const DOM::Utils::MATRIX2D& matrix);

		// Sets a specific key point in a color ramp (for both radial and linear gradient)
		FCM::Result SetKeyColorPoint(
			const DOM::Utils::GRADIENT_COLOR_POINT& colorPoint);

		// End Linear Gradient fill style definition
		FCM::Result EndDefineLinearGradientFillStyle();

		// Start Radial Gradient fill style definition
		FCM::Result StartDefineRadialGradientFillStyle(
			DOM::FillStyle::GradientSpread spread,
			const DOM::Utils::MATRIX2D& matrix,
			FCM::S_Int32 focalPoint);

		// End Radial Gradient fill style definition
		FCM::Result EndDefineRadialGradientFillStyle();

		// Start of fill region boundary
		FCM::Result StartDefineBoundary();

		// Sets a segment of a path (Used for boundary, holes)
		FCM::Result SetSegment(const DOM::Utils::SEGMENT& segment);

		// End of fill region boundary
		FCM::Result EndDefineBoundary();

		// Start of fill region hole
		FCM::Result StartDefineHole();

		// End of fill region hole
		FCM::Result EndDefineHole();

		// Start of stroke group
		FCM::Result StartDefineStrokeGroup();

		// Start solid stroke style definition
		FCM::Result StartDefineSolidStrokeStyle(
			FCM::Double thickness,
			const DOM::StrokeStyle::JOIN_STYLE& joinStyle,
			const DOM::StrokeStyle::CAP_STYLE& capStyle,
			DOM::Utils::ScaleType scaleType,
			FCM::Boolean strokeHinting);

		// End of solid stroke style
		FCM::Result EndDefineSolidStrokeStyle();

		// Start of stroke
		FCM::Result StartDefineStroke();

		// End of a stroke
		FCM::Result EndDefineStroke();

		// End of stroke group
		FCM::Result EndDefineStrokeGroup();

		// End of fill style definition
		FCM::Result EndDefineFill();

		// Marks the end of a shape
		FCM::Result EndDefineShape(FCM::U_Int32 resId);

		// Define a bitmap
		FCM::Result DefineBitmap(
			FCM::U_Int32 resId,
			FCM::S_Int32 height,
			FCM::S_Int32 width,
			const std::string& libPathName,
			DOM::LibraryItem::PIMediaItem pMediaItem);

		// Define text
		FCM::Result DefineText(
			FCM::U_Int32 resId,
			const std::string& name,
			const DOM::Utils::COLOR& color,
			const std::string& displayText,
			DOM::FrameElement::PIClassicText pTextItem);

		FCM::Result DefineSound(
			FCM::U_Int32 resId,
			const std::string& libPathName,
			DOM::LibraryItem::PIMediaItem pMediaItem);
		JSONOutputWriter(FCM::PIFCMCallback pCallback);

		~JSONOutputWriter();

		// Start of a path
		FCM::Result StartDefinePath();

		// End of a path
		FCM::Result EndDefinePath();

	private:

		FCM::Result CreateImageFileName(const std::string& libPathName, std::string& name);

		FCM::Result CreateSoundFileName(const std::string& libPathName, std::string& name);

		FCM::Boolean GetImageExportFileName(const std::string& libPathName, std::string& name);

		void SetImageExportFileName(const std::string& libPathName, const std::string& name);

	private:

		JSONNode* m_pRootNode;

		JSONNode* m_pShapeArray;

		JSONNode* m_pTimelineArray;

		JSONNode* m_pBitmapArray;

		JSONNode* m_pSoundArray;

		JSONNode* m_pTextArray;

		JSONNode* m_shapeElem;

		JSONNode* m_pathArray;

		JSONNode* m_pathElem;

		JSONNode* m_gradientColor;

		JSONNode* m_stopPointArray;

		std::string m_pathCmdStr;

		bool        m_firstSegment;

		STROKE_STYLE m_strokeStyle;

		std::string m_outputJSONFilePath;

		std::string m_outputJSONFileName;

		std::string m_outputImageFolder;

		std::string m_outputSoundFolder;

		FCM::PIFCMCallback m_pCallback;

		FCM::U_Int32 m_imageFileNameLabel;

		FCM::U_Int32 m_soundFileNameLabel;

		std::map<std::string, std::string> m_imageMap;

		FCM::Boolean m_imageFolderCreated;

		FCM::Boolean m_soundFolderCreated;
	};
}