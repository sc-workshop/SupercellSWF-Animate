#pragma once 

#include <FCMTypes.h>
#include <Exporter/Service/ITimelineBuilder2.h>

#include "PluginConfiguration.h"
#include "Writers/Base/OutputWriter.h"
#include "Writers/Base/TimelineWriter.h"

#include "Version.h"

using namespace Exporter::Service;

namespace SupercellSWF {
    class TimelineBuilder : public ITimelineBuilder2, public FCMObjectBase
    {
    public:

        BEGIN_INTERFACE_MAP(TimelineBuilder, PLUGIN_VERSION)
            INTERFACE_ENTRY(ITimelineBuilder2)
        END_INTERFACE_MAP

        virtual FCM::Result _FCMCALL AddShape(
            FCM::U_Int32 objectId,
            SHAPE_INFO* pShapeInfo);

        virtual FCM::Result _FCMCALL AddClassicText(
            FCM::U_Int32 objectId,
            CLASSIC_TEXT_INFO* pClassicTextInfo);

        virtual FCM::Result _FCMCALL AddBitmap(
            FCM::U_Int32 objectId,
            BITMAP_INFO* pBitmapInfo);

        virtual FCM::Result _FCMCALL AddMovieClip(
            FCM::U_Int32 objectId,
            MOVIE_CLIP_INFO* pMovieClipInfo,
            DOM::FrameElement::PIMovieClip pMovieClip);

        virtual FCM::Result _FCMCALL AddGraphic(
            FCM::U_Int32 objectId,
            GRAPHIC_INFO* pGraphicInfo);

        virtual FCM::Result _FCMCALL AddSound(
            FCM::U_Int32 objectId,
            SOUND_INFO* pSoundInfo,
            DOM::FrameElement::PISound pSound);

        virtual FCM::Result _FCMCALL UpdateZOrder(
            FCM::U_Int32 objectId,
            FCM::U_Int32 placeAfterObjectId);

        virtual FCM::Result UpdateMask(
            FCM::U_Int32 objectId,
            FCM::U_Int32 maskTillObjectId);

        virtual FCM::Result _FCMCALL Remove(FCM::U_Int32 objectId);

        virtual FCM::Result _FCMCALL UpdateBlendMode(
            FCM::U_Int32 objectId,
            DOM::FrameElement::BlendMode blendMode);

        virtual FCM::Result _FCMCALL UpdateVisibility(
            FCM::U_Int32 objectId,
            FCM::Boolean visible);

        virtual FCM::Result _FCMCALL UpdateGraphicFilter(
            FCM::U_Int32 objectId,
            PIFCMList pFilterable);

        virtual FCM::Result _FCMCALL UpdateDisplayTransform(
            FCM::U_Int32 objectId,
            const DOM::Utils::MATRIX2D& matrix);

        virtual FCM::Result _FCMCALL UpdateColorTransform(
            FCM::U_Int32 objectId,
            const DOM::Utils::COLOR_MATRIX& colorMatrix);

        virtual FCM::Result _FCMCALL ShowFrame();

        virtual FCM::Result _FCMCALL AddFrameScript(FCM::CStringRep16 pScript, FCM::U_Int32 layerNum);

        virtual FCM::Result _FCMCALL RemoveFrameScript(FCM::U_Int32 layerNum);

        virtual FCM::Result _FCMCALL SetFrameLabel(FCM::StringRep16 pLabel, DOM::KeyFrameLabelType labelType);

        TimelineBuilder();

        ~TimelineBuilder();

        virtual FCM::Result Build(
            FCM::U_Int32 resourceId,
            FCM::StringRep16 pName,
            TimelineWriter** ppTimelineWriter);

        void Init(OutputWriter* pOutputWriter);

    private:

        OutputWriter* m_pOutputWriter;

        TimelineWriter* m_pTimelineWriter;

        FCM::U_Int32 m_frameIndex;
    };
}