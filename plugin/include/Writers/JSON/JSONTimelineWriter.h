#pragma once

#include <FCMTypes.h>
#include <Utils/DOMTypes.h>

#include "libjson/libjson.h"

#include "Writers/Base/TimelineWriter.h"

namespace SupercellSWF {
    class JSONTimelineWriter : public TimelineWriter
    {
    public:

        FCM::Result PlaceObject(
            FCM::U_Int32 resId,
            FCM::U_Int32 objectId,
            FCM::U_Int32 placeAfterObjectId,
            const DOM::Utils::MATRIX2D* pMatrix,
            FCM::PIFCMUnknown pUnknown = NULL);
        FCM::Result PlaceObject(
            FCM::U_Int32 resId,
            FCM::U_Int32 objectId,
            FCM::PIFCMUnknown pUnknown = NULL);

        FCM::Result RemoveObject(
            FCM::U_Int32 objectId);

        FCM::Result UpdateZOrder(
            FCM::U_Int32 objectId,
            FCM::U_Int32 placeAfterObjectId);

        FCM::Result UpdateMask(
            FCM::U_Int32 objectId,
            FCM::U_Int32 maskTillObjectId);

        FCM::Result UpdateBlendMode(
            FCM::U_Int32 objectId,
            DOM::FrameElement::BlendMode blendMode);

        FCM::Result UpdateVisibility(
            FCM::U_Int32 objectId,
            FCM::Boolean visible);

        FCM::Result AddGraphicFilter(
            FCM::U_Int32 objectId,
            FCM::PIFCMUnknown pFilter);

        FCM::Result UpdateDisplayTransform(
            FCM::U_Int32 objectId,
            const DOM::Utils::MATRIX2D& matrix);

        FCM::Result UpdateColorTransform(
            FCM::U_Int32 objectId,
            const DOM::Utils::COLOR_MATRIX& colorMatrix);

        FCM::Result ShowFrame(FCM::U_Int32 frameNum);

        FCM::Result AddFrameScript(FCM::CStringRep16 pScript, FCM::U_Int32 layerNum);

        FCM::Result RemoveFrameScript(FCM::U_Int32 layerNum);

        FCM::Result SetFrameLabel(FCM::StringRep16 pLabel, DOM::KeyFrameLabelType labelType);

        JSONTimelineWriter(FCM::PIFCMCallback pCallback);

        virtual ~JSONTimelineWriter();

        const JSONNode* GetRoot();

        void Finish(FCM::U_Int32 resId, FCM::StringRep16 pName);

    private:

        JSONNode* m_pCommandArray;

        JSONNode* m_pFrameArray;

        JSONNode* m_pTimelineElement;

        JSONNode* m_pFrameElement;

        FCM::PIFCMCallback m_pCallback;
    };
}