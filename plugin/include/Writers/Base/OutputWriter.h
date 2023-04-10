#pragma once

#include <FCMTypes.h>
#include <Utils/DOMTypes.h>
#include <FillStyle/IGradientFillStyle.h>
#include <StrokeStyle/ISolidStrokeStyle.h>
#include <LIbraryItem/IMediaItem.h>
#include <FrameElement/IClassicText.h>

#include <string>

#include "TimelineWriter.h"

namespace SupercellSWF {

    enum StrokeStyleType
    {
        INVALID_STROKE_STYLE_TYPE,
        SOLID_STROKE_STYLE_TYPE
    };

    struct SOLID_STROKE_STYLE
    {
        FCM::Double thickness;
        DOM::StrokeStyle::JOIN_STYLE joinStyle;
        DOM::StrokeStyle::CAP_STYLE capStyle;
        DOM::Utils::ScaleType scaleType;
        FCM::Boolean strokeHinting;
    };

    struct STROKE_STYLE
    {
        StrokeStyleType type;

        union
        {
            SOLID_STROKE_STYLE solidStrokeStyle;
        };
    };

    class OutputWriter
    {
    public:

        // Marks the begining of the output
        virtual FCM::Result StartOutput(std::string& outputFileName) = 0;

        // Marks the end of the output
        virtual FCM::Result EndOutput() = 0;

        // Marks the begining of the Document
        virtual FCM::Result StartDocument(
            const DOM::Utils::COLOR& background,
            FCM::U_Int32 stageHeight,
            FCM::U_Int32 stageWidth,
            FCM::U_Int32 fps) = 0;

        // Marks the end of the Document
        virtual FCM::Result EndDocument() = 0;

        // Marks the start of a timeline
        virtual FCM::Result StartDefineTimeline() = 0; // TODO change to defineMovieClip

        // Marks the end of a timeline
        virtual FCM::Result EndDefineTimeline(
            FCM::U_Int32 resId,
            FCM::StringRep16 pName,
            TimelineWriter* pTimelineWriter) = 0;

        // Marks the start of a shape
        virtual FCM::Result StartDefineShape() = 0;

        // Start of fill region definition
        virtual FCM::Result StartDefineFill() = 0;

        // Solid fill style definition
        virtual FCM::Result DefineSolidFillStyle(const DOM::Utils::COLOR& color) = 0;

        // Bitmap fill style definition
        virtual FCM::Result DefineBitmapFillStyle(
            FCM::Boolean clipped,
            const DOM::Utils::MATRIX2D& matrix,
            FCM::S_Int32 height,
            FCM::S_Int32 width,
            const std::string& libPathName,
            DOM::LibraryItem::PIMediaItem pMediaItem) = 0;

        // Start Linear Gradient fill style definition
        virtual FCM::Result StartDefineLinearGradientFillStyle(
            DOM::FillStyle::GradientSpread spread,
            const DOM::Utils::MATRIX2D& matrix) = 0;

        // Sets a specific key point in a color ramp (for both radial and linear gradient)
        virtual FCM::Result SetKeyColorPoint(
            const DOM::Utils::GRADIENT_COLOR_POINT& colorPoint) = 0;

        // End Linear Gradient fill style definition
        virtual FCM::Result EndDefineLinearGradientFillStyle() = 0;

        // Start Radial Gradient fill style definition
        virtual FCM::Result StartDefineRadialGradientFillStyle(
            DOM::FillStyle::GradientSpread spread,
            const DOM::Utils::MATRIX2D& matrix,
            FCM::S_Int32 focalPoint) = 0;

        // End Radial Gradient fill style definition
        virtual FCM::Result EndDefineRadialGradientFillStyle() = 0;

        // Start of fill region boundary
        virtual FCM::Result StartDefineBoundary() = 0;

        // Sets a segment of a path (Used for boundary, holes)
        virtual FCM::Result SetSegment(const DOM::Utils::SEGMENT& segment) = 0;

        // End of fill region boundary
        virtual FCM::Result EndDefineBoundary() = 0;

        // Start of fill region hole
        virtual FCM::Result StartDefineHole() = 0;

        // End of fill region hole
        virtual FCM::Result EndDefineHole() = 0;

        // Start of stroke group
        virtual FCM::Result StartDefineStrokeGroup() = 0;

        // Start solid stroke style definition
        virtual FCM::Result StartDefineSolidStrokeStyle(
            FCM::Double thickness,
            const DOM::StrokeStyle::JOIN_STYLE& joinStyle,
            const DOM::StrokeStyle::CAP_STYLE& capStyle,
            DOM::Utils::ScaleType scaleType,
            FCM::Boolean strokeHinting) = 0;

        // End of solid stroke style
        virtual FCM::Result EndDefineSolidStrokeStyle() = 0;

        // Start of stroke 
        virtual FCM::Result StartDefineStroke() = 0;

        // End of a stroke 
        virtual FCM::Result EndDefineStroke() = 0;

        // End of stroke group
        virtual FCM::Result EndDefineStrokeGroup() = 0;

        // End of fill style definition
        virtual FCM::Result EndDefineFill() = 0;

        // Marks the end of a shape
        virtual FCM::Result EndDefineShape(FCM::U_Int32 resId) = 0;

        // Define a bitmap
        virtual FCM::Result DefineBitmap(
            FCM::U_Int32 resId,
            FCM::S_Int32 height,
            FCM::S_Int32 width,
            const std::string& libPathName,
            DOM::LibraryItem::PIMediaItem pMediaItem) = 0;

        // Define a classic text
        virtual FCM::Result DefineText(
            FCM::U_Int32 resId,
            const std::string& name,
            const DOM::Utils::COLOR& color,
            const std::string& displayText,
            DOM::FrameElement::PIClassicText pTextItem) = 0;

        // Define Sound
        virtual FCM::Result DefineSound(
            FCM::U_Int32 resId,
            const std::string& libPathName,
            DOM::LibraryItem::PIMediaItem pMediaItem) = 0;
    };
}