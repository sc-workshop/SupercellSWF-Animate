#include "Publisher/FrameGenerator.h"

#include "Publisher/ResourcePublisher.h"
#include "Publisher/Shared/SharedMovieclipWriter.h"

#include "DOM/FrameElement/ISymbolInstance.h"

namespace sc {
	namespace Adobe {

		Result FrameGenerator::InitializeService() {
			Result res = FCM_SUCCESS;

			if (!TweenerService)
			{
				FCM::AutoPtr<FCM::IFCMUnknown> unk;
				res = m_callback->GetService(TWEENER_SERVICE, unk.m_Ptr);
				FCM_CHECK;

				TweenerService = unk;
				if (!TweenerService) {
					return FCM_SERVICE_NOT_FOUND;
				}
			}

			return res;
		}

		Result FrameGenerator::GetDisplayInstanceData(
			DOM::FrameElement::IFrameDisplayElement* element,
			uint16_t& identifer,
			uint8_t& blending,
			std::string& name
		) {
			Result res = FCM_SUCCESS;
			// Trying to get frame element type
			AutoPtr<DOM::FrameElement::IInstance> libraryItemElement = element;

			// If frame element is Symbol or Bitmap
			if (libraryItemElement) {
				DOM::PILibraryItem libraryItem;
				res = libraryItemElement->GetLibraryItem(libraryItem);
				FCM_CHECK;

				StringRep16 libraryItemName;
				res = libraryItem->GetName(&libraryItemName);
				FCM_CHECK;

				std::string libraryItemName_str = Utils::ToString(libraryItemName, m_callback);

				res = m_resources->GetIdentifer(libraryItemName_str, identifer);
				FCM_CHECK;

				// If Item not exported yet
				if (identifer == UINT16_MAX) {
					res = m_resources->AddLibraryItem(libraryItem, identifer);
					FCM_CHECK;
				}
			}
			else {
				return FCM_EXPORT_FAILED;
			}

			// TODO: Blending
			blending = 0;

			return res;
		}

		Result FrameGenerator::GenerateTweenFrame(
			pSharedMovieclipWriter writer,
			DOM::ITween* tween,
			DOM::FrameElement::IFrameDisplayElement* element,
			uint32_t duration,
			uint32_t& frameOffset
		) {
			Result res;
			FCM::AutoPtr<FCM::IFCMUnknown> unknownTweener;

			// Tweeners
			bool hasMatrixTransform = false;
			AutoPtr<IGeometricTweener> matrixTweener = nullptr;

			bool hasColorTransform = false;
			AutoPtr<IColorTweener> colorTweener = nullptr;

			// Tween guids dict
			PIFCMDictionary tweenerDict;
			res = tween->GetTweenedProperties(tweenerDict);
			FCM_CHECK;

			// Matrix transformation
			FCMGUID matrixGuid;
			hasMatrixTransform = Utils::ReadGUID(tweenerDict, kDOMGeometricProperty, matrixGuid);

			// Color transformation 
			FCMGUID colorGuid;
			hasColorTransform = Utils::ReadGUID(tweenerDict, kDOMColorProperty, colorGuid);

			//Debug stuff
			debugLog("Has matrix: %d", hasMatrixTransform);
			debugLog("Has color: %d", hasColorTransform);

			// Tweener initialize
			if (hasMatrixTransform) {
				res = TweenerService->GetTweener(matrixGuid, nullptr, unknownTweener.m_Ptr);
				matrixTweener = unknownTweener;
				unknownTweener.m_Ptr = nullptr;
				FCM_CHECK;
			}
			if (hasColorTransform) {
				res = TweenerService->GetTweener(colorGuid, nullptr, unknownTweener.m_Ptr);
				colorTweener = unknownTweener;
				unknownTweener.m_Ptr = nullptr;
				FCM_CHECK;
			}

			uint16_t instanceIdentifer;
			U_Int8 instanceBlending;
			std::string instanceName;
			res = GetDisplayInstanceData(element, instanceIdentifer, instanceBlending, instanceName);
			FCM_CHECK;

			DOM::Utils::MATRIX2D baseMatrix({1, 0, 0, 1, 0, 0});
			res = element->GetMatrix(baseMatrix);
			FCM_CHECK;

			// DOM::Utils::COLOR_MATRIX baseColor{
			// 	1, 0, 0, 0, 0,
			// 	0, 1, 0, 0, 0,
			// 	0, 0, 1, 0, 0,
			// 	0, 0, 0, 1, 0
			// };
			// AutoPtr<DOM::FrameElement::ISymbolInstance> symbolInstance = element;
			// if (symbolInstance) {
			// 	symbolInstance->GetColorMatrix(baseColor);
			// }

			for (uint32_t durationIndex = 0; duration > durationIndex; durationIndex++) {
				DOM::Utils::MATRIX2D matrix(baseMatrix);
				DOM::Utils::COLOR_MATRIX color{
				1, 0, 0, 0, 0,
				0, 1, 0, 0, 0,
				0, 0, 1, 0, 0,
				0, 0, 0, 1, 0
				};

				if (hasMatrixTransform) {
					DOM::Utils::MATRIX2D transformMatrix;
					matrixTweener->GetGeometricTransform(tween, durationIndex, transformMatrix);

					matrix.a = transformMatrix.a * baseMatrix.a + transformMatrix.c * baseMatrix.b;
					matrix.d = transformMatrix.d * baseMatrix.d + transformMatrix.b * baseMatrix.c;

					matrix.b = baseMatrix.a * transformMatrix.b + baseMatrix.b * transformMatrix.d;
					matrix.c = baseMatrix.c * transformMatrix.a + baseMatrix.d * transformMatrix.c;

					matrix.tx = transformMatrix.a * baseMatrix.tx + transformMatrix.c * baseMatrix.ty + transformMatrix.tx;
					matrix.ty = transformMatrix.b * baseMatrix.tx + transformMatrix.d * baseMatrix.ty + transformMatrix.ty;
				}

				if (hasColorTransform) {
					//DOM::Utils::COLOR_MATRIX transformColor;
					colorTweener->GetColorMatrix(tween, durationIndex, color);
				}

				res = writer->AddFrameElement(
					frameOffset,
					instanceIdentifer,
					instanceBlending,
					instanceName,
					matrix,
					color
				);
				FCM_CHECK;

				frameOffset++;
			}

			return res;
		}

		Result FrameGenerator::GenerateFrame(
			pSharedMovieclipWriter writer,
			FCM::FCMListPtr frameElements,
			uint32_t frameElementsCount,
			uint32_t duration,
			uint32_t& frameOffset
		) {
			Result res = FCM_SUCCESS;

			uint32_t i = frameElementsCount;
			for (uint32_t elementIndex = 0; frameElementsCount > elementIndex; elementIndex++) {
				AutoPtr<DOM::FrameElement::IFrameDisplayElement> frameElement = frameElements[--i];

				uint16_t instanceIdentifer;
				U_Int8 instanceBlending;
				std::string instanceName;
				res = GetDisplayInstanceData(frameElement, instanceIdentifer, instanceBlending, instanceName);
				FCM_CHECK;

				// Frames stuff
				DOM::Utils::MATRIX2D transformMatrix;
				frameElement->GetMatrix(transformMatrix);

				DOM::Utils::COLOR_MATRIX colorMatrix;
				AutoPtr<DOM::FrameElement::ISymbolInstance> symbolInstance = frameElement;
				if (symbolInstance) {
					symbolInstance->GetColorMatrix(colorMatrix);
				}

				// Iterate through frame duration
				for (U_Int32 durationOffset = 0; duration > durationOffset; durationOffset++) {
					writer->AddFrameElement(
						frameOffset + durationOffset,
						instanceIdentifer,
						instanceBlending,
						instanceName,
						transformMatrix,
						colorMatrix
					);
				}
			}

			frameOffset += duration;

			return res;
		}

		Result FrameGenerator::GenerateLayerFrames(
			pSharedMovieclipWriter writer,
			AutoPtr<DOM::Layer::ILayerNormal> layer
		) {
			Result res = FCM_SUCCESS;

			// Current frame position on timeline
			U_Int32 frameOffset = 0;

			// Layer max frame count
			uint32_t layerDuration;
			res = layer->GetTotalDuration(layerDuration);
			FCM_CHECK;

			FCM::FCMListPtr keyframes;
			res = layer->GetKeyFrames(keyframes.m_Ptr);
			FCM_CHECK;

			uint32_t keyframesCount = 0;
			res = keyframes->Count(keyframesCount);
			FCM_CHECK;

			debugLog("Layer duration: %d, keyframes: %d", layerDuration, keyframesCount);

			// Iterate through layer keys
			for (uint32_t keyframeIndex = 0; keyframesCount > keyframeIndex; keyframeIndex++) {
				AutoPtr<DOM::IFrame> keyframe = keyframes[keyframeIndex];

				uint32_t keyframeDuration = 0;
				res = keyframe->GetDuration(keyframeDuration);
				FCM_CHECK;

				AutoPtr<DOM::ITween> tween;
				res = keyframe->GetTween(tween.m_Ptr);
				FCM_CHECK;

				FCM::FCMListPtr frameElements;
				res = keyframe->GetFrameElements(frameElements.m_Ptr);
				FCM_CHECK;

				U_Int32 frameElementsCount = 0;
				res = frameElements->Count(frameElementsCount);
				FCM_CHECK;

				debugLog("Keyframe %d. Duration: %d. Elements: %d", keyframeIndex, keyframeDuration, frameElementsCount);

				// if keyframe tweened
				if (tween && frameElementsCount == 1) {
					debugLog("Keyframe %d has tween", keyframeIndex);

					AutoPtr<DOM::FrameElement::IFrameDisplayElement> tweenedElement = frameElements[0];

					res = GenerateTweenFrame(writer, tween, tweenedElement, keyframeDuration, frameOffset);
					FCM_CHECK;
				}
				else // Process as regular keyframe
				{
					res = GenerateFrame(writer, frameElements, frameElementsCount, keyframeDuration, frameOffset);
					FCM_CHECK;
				}
			}

			return res;
		}

		Result FrameGenerator::GenerateLayer(
			pSharedMovieclipWriter writer,
			AutoPtr<DOM::ILayer2> layer
		) {
			Result res = FCM_SUCCESS;

			AutoPtr<IFCMUnknown> unknownLayer;
			layer->GetLayerType(unknownLayer.m_Ptr);

			AutoPtr<DOM::Layer::ILayerNormal> normalLayer = unknownLayer;
			AutoPtr<DOM::Layer::ILayerMask> maskLayer = unknownLayer;
			AutoPtr<DOM::Layer::ILayerGuide> guideLayer = unknownLayer;
			AutoPtr<DOM::Layer::ILayerFolder> folderLayer = unknownLayer;

			if (folderLayer) {
				FCM::FCMListPtr folderLayers;
				res = folderLayer->GetChildren(folderLayers.m_Ptr);
				FCM_CHECK;
				res = GenerateLayerList(writer, folderLayers);
				return res;
			}
			else if (guideLayer) {
				// TODO: i need to see how it works
				return res;
			}
			else if (maskLayer) {
				// TODO: masks, for now export as usual

				// layer mask should be the same as normal layer
				if (!normalLayer) {
					return FCM_EXPORT_FAILED;
				}
				else {
					res = GenerateLayer(writer, normalLayer);
					FCM_CHECK;
				}

				FCM::FCMListPtr maskedLayers;
				res = maskLayer->GetChildren(maskedLayers.m_Ptr);
				FCM_CHECK;

				res = GenerateLayerList(writer, maskedLayers);
				return res;
			}
			else if (normalLayer) {
				res = GenerateLayerFrames(writer, normalLayer);
				return res;
			}

			// Layer must pass at least one of conditions above
			return FCM_EXPORT_FAILED;
		}

		Result FrameGenerator::GenerateLayerList(
			pSharedMovieclipWriter writer,
			FCMListPtr layers
		) {
			Result res = FCM_SUCCESS;

			U_Int32 layerCount = 0;
			res = layers->Count(layerCount);
			FCM_CHECK;

			uint32_t i = layerCount;
			for (U_Int32 layerIndex = 0; layerCount > layerIndex; layerIndex++) {
				AutoPtr<DOM::ILayer2> layer = layers[--i];

#ifdef DEBUG
				StringRep16 layerName;
				layer->GetName(&layerName);
				debugLog("|------------------------------ %s -------------------------------|", Utils::ToString(layerName, m_callback).c_str());
#endif // DEBUG

				res = GenerateLayer(writer, layer);
				FCM_CHECK;
			};

			return res;
		}

		Result FrameGenerator::Generate(pSharedMovieclipWriter writer, DOM::ITimeline* timeline) {
			Result res;

			res = InitializeService();
			FCM_CHECK;

			// Timeline duration
			U_Int32 framesCount = 0;
			res = timeline->GetMaxFrameCount(framesCount);
			FCM_CHECK;

			debugLog("Symbol duration: %d", framesCount);

			//Initialize frames
			res = writer->InitTimeline(framesCount);
			FCM_CHECK;

			// All layers in symbol
			FCM::FCMListPtr layers;
			res = timeline->GetLayers(layers.m_Ptr);
			FCM_CHECK;

			res = GenerateLayerList(writer, layers);

			return res;
		}

	}
}