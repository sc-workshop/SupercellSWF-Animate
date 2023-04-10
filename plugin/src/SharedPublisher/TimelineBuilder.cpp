#include "SharedPublisher/TimelineBuilder.h"

#include "MediaInfo/IBitmapInfo.h"
#include "FrameElement/IBitmapInstance.h"
#include "FrameElement/ISound.h"
#include "MediaInfo/ISoundInfo.h"
#include "LibraryItem/IMediaItem.h"
#include "LibraryItem/IFolderItem.h"
#include "LibraryItem/IFontItem.h"
#include "LibraryItem/ISymbolItem.h"
#include "ILibraryItem.h"

#include "FrameElement/IButton.h"

#include "Writers/JSON/JSONTimelineWriter.h"
#include "Utils.h"

namespace SupercellSWF {
	FCM::Result TimelineBuilder::AddShape(FCM::U_Int32 objectId, SHAPE_INFO* pShapeInfo)
	{
		FCM::Result res;

		ASSERT(pShapeInfo);
		ASSERT(pShapeInfo->structSize >= sizeof(SHAPE_INFO));

		console.debugLog("{AddShape} ObjId: %d ResId: %d PlaceAfter: %d",
			objectId, pShapeInfo->resourceId, pShapeInfo->placeAfterObjectId);

		res = m_timelineWriter->PlaceObject(
			pShapeInfo->resourceId,
			objectId,
			pShapeInfo->placeAfterObjectId,
			&pShapeInfo->matrix);

		return res;
	}

	FCM::Result TimelineBuilder::AddClassicText(FCM::U_Int32 objectId, CLASSIC_TEXT_INFO* pClassicTextInfo)
	{
		FCM::Result res;

		ASSERT(pClassicTextInfo);
		ASSERT(pClassicTextInfo->structSize >= sizeof(CLASSIC_TEXT_INFO));

		console.debugLog("{AddClassicText} ObjId: %d ResId: %d PlaceAfter: %d",
			objectId, pClassicTextInfo->resourceId, pClassicTextInfo->placeAfterObjectId);

		//To get the bounding rect of the text
		if (pClassicTextInfo->structSize >= sizeof(DISPLAY_OBJECT_INFO_2))
		{
			DOM::Utils::RECT rect;
			DISPLAY_OBJECT_INFO_2* ptr = static_cast<DISPLAY_OBJECT_INFO_2*>(pClassicTextInfo);
			if (ptr)
			{
				rect = ptr->bounds;
				// This rect object gives the bound of the text filed.
				// This will have to be transformed using the pClassicTextInfo->matrix
				// to map it to its parent's co-orinate space to render it.
			}
		}

		res = m_timelineWriter->PlaceObject(
			pClassicTextInfo->resourceId,
			objectId,
			pClassicTextInfo->placeAfterObjectId,
			&pClassicTextInfo->matrix);

		return res;
	}

	FCM::Result TimelineBuilder::AddBitmap(FCM::U_Int32 objectId, BITMAP_INFO* pBitmapInfo)
	{
		FCM::Result res;

		ASSERT(pBitmapInfo);
		ASSERT(pBitmapInfo->structSize >= sizeof(BITMAP_INFO));

		console.debugLog("{AddBitmap} ObjId: %d ResId: %d PlaceAfter: %d",
			objectId, pBitmapInfo->resourceId, pBitmapInfo->placeAfterObjectId);

		res = m_timelineWriter->PlaceObject(
			pBitmapInfo->resourceId,
			objectId,
			pBitmapInfo->placeAfterObjectId,
			&pBitmapInfo->matrix);

		return res;
	}

	FCM::Result TimelineBuilder::AddMovieClip(FCM::U_Int32 objectId, MOVIE_CLIP_INFO* pMovieClipInfo, DOM::FrameElement::PIMovieClip pMovieClip)
	{
		FCM::Result res;
		FCM::AutoPtr<FCM::IFCMUnknown> pUnknown = pMovieClip;

		ASSERT(pMovieClipInfo);
		ASSERT(pMovieClipInfo->structSize >= sizeof(MOVIE_CLIP_INFO));

		console.debugLog("{AddMovieClip} ObjId: %d ResId: %d PlaceAfter: %d",
			objectId, pMovieClipInfo->resourceId, pMovieClipInfo->placeAfterObjectId);

		AutoPtr<DOM::FrameElement::IButton> pButton = pMovieClip;
		if (pButton.m_Ptr)
		{
			DOM::FrameElement::ButtonTrackMode trackMode;
			pButton->GetTrackingMode(trackMode);
			if (trackMode == DOM::FrameElement::TRACK_AS_BUTTON)
			{
				console.debugLog("{AddMovieClip} ObjId: %d, is a button with TrackingMode set to TRACK_AS_BUTTON",
					objectId);

			}
			else
			{
				console.debugLog("{AddMovieClip} ObjId: %d, is a button with TrackingMode set to TRACK_AS_MENU_ITEM",
					objectId);
			}
		}

		res = m_timelineWriter->PlaceObject(
			pMovieClipInfo->resourceId,
			objectId,
			pMovieClipInfo->placeAfterObjectId,
			&pMovieClipInfo->matrix,
			pUnknown);

		return res;
	}

	FCM::Result TimelineBuilder::AddGraphic(FCM::U_Int32 objectId, GRAPHIC_INFO* pGraphicInfo)
	{
		FCM::Result res;

		ASSERT(pGraphicInfo);
		ASSERT(pGraphicInfo->structSize >= sizeof(GRAPHIC_INFO));

		console.debugLog("{AddGraphic} ObjId: %d ResId: %d PlaceAfter: %d",
			objectId, pGraphicInfo->resourceId, pGraphicInfo->placeAfterObjectId);

		res = m_timelineWriter->PlaceObject(
			pGraphicInfo->resourceId,
			objectId,
			pGraphicInfo->placeAfterObjectId,
			&pGraphicInfo->matrix);

		return res;
	}

	FCM::Result TimelineBuilder::AddSound(
		FCM::U_Int32 objectId,
		SOUND_INFO* pSoundInfo,
		DOM::FrameElement::PISound pSound)
	{
		FCM::AutoPtr<FCM::IFCMUnknown> pUnknown = pSound;
		FCM::Result res;

		ASSERT(pSoundInfo);
		ASSERT(pSoundInfo->structSize == sizeof(SOUND_INFO));

		console.debugLog("{AddSound} ObjId: %d ResId: %d",
			objectId, pSoundInfo->resourceId);

		res = m_timelineWriter->PlaceObject(
			pSoundInfo->resourceId,
			objectId,
			pUnknown);

		return res;
	}

	FCM::Result TimelineBuilder::UpdateZOrder(FCM::U_Int32 objectId, FCM::U_Int32 placeAfterObjectId)
	{
		FCM::Result res = FCM_SUCCESS;

		console.debugLog("{UpdateZOrder} ObjId: %d PlaceAfter: %d",
			objectId, placeAfterObjectId);

		res = m_timelineWriter->UpdateZOrder(objectId, placeAfterObjectId);

		return res;
	}

	FCM::Result TimelineBuilder::UpdateMask(FCM::U_Int32 objectId, FCM::U_Int32 maskTillObjectId)
	{
		FCM::Result res = FCM_SUCCESS;

		console.debugLog("{UpdateMask} ObjId: %d MaskTill: %d",
			objectId, maskTillObjectId);

		res = m_timelineWriter->UpdateMask(objectId, maskTillObjectId);

		return res;
	}

	FCM::Result TimelineBuilder::Remove(FCM::U_Int32 objectId)
	{
		FCM::Result res;

		console.debugLog("{Remove} ObjId: %d", objectId);

		res = m_timelineWriter->RemoveObject(objectId);

		return res;
	}

	FCM::Result TimelineBuilder::UpdateBlendMode(FCM::U_Int32 objectId, DOM::FrameElement::BlendMode blendMode)
	{
		FCM::Result res;

		console.debugLog("{UpdateBlendMode} ObjId: %d BlendMode: %d", objectId, blendMode);

		res = m_timelineWriter->UpdateBlendMode(objectId, blendMode);

		return res;
	}

	FCM::Result TimelineBuilder::UpdateVisibility(FCM::U_Int32 objectId, FCM::Boolean visible)
	{
		FCM::Result res;

		console.debugLog("{UpdateVisibility} ObjId: %d Visible: %d", objectId, visible);

		res = m_timelineWriter->UpdateVisibility(objectId, visible);

		return res;
	}


	FCM::Result TimelineBuilder::UpdateGraphicFilter(FCM::U_Int32 objectId, PIFCMList pFilterable)
	{
		FCM::U_Int32 count;
		FCM::Result res;
		FCM::FCMListPtr pFilterList;

		console.debugLog("{UpdateGraphicFilter} ObjId: %d", objectId);

		res = pFilterable->Count(count);
		ASSERT(FCM_SUCCESS_CODE(res));

		for (FCM::U_Int32 i = 0; i < count; i++)
		{
			FCM::AutoPtr<FCM::IFCMUnknown> pUnknown = (*pFilterable)[i];
			res = m_timelineWriter->AddGraphicFilter(objectId, pUnknown.m_Ptr);

			if (FCM_FAILURE_CODE(res))
			{
				return res;
			}
		}

		return FCM_SUCCESS;
	}


	FCM::Result TimelineBuilder::UpdateDisplayTransform(FCM::U_Int32 objectId, const DOM::Utils::MATRIX2D& matrix)
	{
		FCM::Result res;

		console.debugLog("{UpdateDisplayTransform} ObjId: %d", objectId);

		res = m_timelineWriter->UpdateDisplayTransform(objectId, matrix);

		return res;
	}

	FCM::Result TimelineBuilder::UpdateColorTransform(FCM::U_Int32 objectId, const DOM::Utils::COLOR_MATRIX& colorMatrix)
	{
		FCM::Result res;

		console.debugLog("{UpdateColorTransform} ObjId: %d", objectId);

		res = m_timelineWriter->UpdateColorTransform(objectId, colorMatrix);

		return res;
	}

	FCM::Result TimelineBuilder::ShowFrame()
	{
		FCM::Result res;

		console.debugLog("{ShowFrame} Frame: %d", m_frameIndex);

		res = m_timelineWriter->ShowFrame(m_frameIndex);

		m_frameIndex++;

		return res;
	}

	FCM::Result TimelineBuilder::AddFrameScript(FCM::CStringRep16 pScript, FCM::U_Int32 layerNum)
	{
		FCM::Result res = FCM_SUCCESS;

		console.debugLog("{AddFrameScript} LayerNum: %d", layerNum);

		if (pScript != NULL)
		{
			res = m_timelineWriter->AddFrameScript(pScript, layerNum);
		}

		return res;
	}

	FCM::Result TimelineBuilder::RemoveFrameScript(FCM::U_Int32 layerNum)
	{
		FCM::Result res = FCM_SUCCESS;

		console.debugLog("{RemoveFrameScript} LayerNum: %d", layerNum);

		res = m_timelineWriter->RemoveFrameScript(layerNum);

		return res;
	}

	FCM::Result TimelineBuilder::SetFrameLabel(FCM::StringRep16 pLabel, DOM::KeyFrameLabelType labelType)
	{
		FCM::Result res = FCM_SUCCESS;

		console.debugLog("{SetFrameLabel}");

		if (pLabel != NULL)
		{
			res = m_timelineWriter->SetFrameLabel(pLabel, labelType);
		}

		return res;
	}

	FCM::Result TimelineBuilder::Build(
		FCM::U_Int32 resourceId,
		FCM::StringRep16 pName,
		TimelineWriter** ppTimelineWriter)
	{
		FCM::Result res;

		res = m_outputWriter->EndDefineTimeline(resourceId, pName, m_timelineWriter);

		*ppTimelineWriter = m_timelineWriter;

		return res;
	}


	TimelineBuilder::TimelineBuilder() :
		m_outputWriter(NULL),
		m_frameIndex(0)
	{
		console.debugLog("{CreateTimeline}");
	}

	TimelineBuilder::~TimelineBuilder()
	{
	}

	void TimelineBuilder::Init(OutputWriter* outputWriter, TimelineWriter* timelineWriter)
	{
		m_outputWriter = outputWriter;
		m_timelineWriter = timelineWriter;
		console.Init("Timeline", GetCallback());

		m_outputWriter->StartDefineTimeline();
	}
}