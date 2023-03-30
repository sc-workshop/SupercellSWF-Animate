#pragma once

#include <FCMTypes.h>
#include <Utils/DOMTypes.h>
#include <FrameElement/IMovieClip.h>
#include <IFrame.h>

#include <cstddef>

namespace SupercellSWF {
    class TimelineWriter
    {
    public:

        virtual FCM::Result PlaceObject(
            FCM::U_Int32 resId,
            FCM::U_Int32 objectId,
            FCM::U_Int32 placeAfterObjectId,
            const DOM::Utils::MATRIX2D* pMatrix,
            FCM::PIFCMUnknown pUnknown = NULL) = 0;

        virtual FCM::Result PlaceObject(
            FCM::U_Int32 resId,
            FCM::U_Int32 objectId,
            FCM::PIFCMUnknown pUnknown = NULL) = 0;

        virtual FCM::Result RemoveObject(
            FCM::U_Int32 objectId) = 0;

        virtual FCM::Result UpdateZOrder(
            FCM::U_Int32 objectId,
            FCM::U_Int32 placeAfterObjectId) = 0;

        virtual FCM::Result UpdateMask(
            FCM::U_Int32 objectId,
            FCM::U_Int32 maskTillObjectId) = 0;

        virtual FCM::Result UpdateBlendMode(
            FCM::U_Int32 objectId,
            DOM::FrameElement::BlendMode blendMode) = 0;

        virtual FCM::Result UpdateVisibility(
            FCM::U_Int32 objectId,
            FCM::Boolean visible) = 0;

        virtual FCM::Result AddGraphicFilter(
            FCM::U_Int32 objectId,
            FCM::PIFCMUnknown pFilter) = 0;

        virtual FCM::Result UpdateDisplayTransform(
            FCM::U_Int32 objectId,
            const DOM::Utils::MATRIX2D& matrix) = 0;

        virtual FCM::Result UpdateColorTransform(
            FCM::U_Int32 objectId,
            const DOM::Utils::COLOR_MATRIX& colorMatrix) = 0;

        virtual FCM::Result ShowFrame(FCM::U_Int32 frameNum) = 0;

        virtual FCM::Result AddFrameScript(FCM::CStringRep16 pScript, FCM::U_Int32 layerNum) = 0;

        virtual FCM::Result RemoveFrameScript(FCM::U_Int32 layerNum) = 0;

        virtual FCM::Result SetFrameLabel(FCM::StringRep16 pLabel, DOM::KeyFrameLabelType labelType) = 0;
    };
}