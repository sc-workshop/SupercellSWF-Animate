#include "Publisher/FrameGenerator.h"

#include "Publisher/ResourcePublisher.h"
#include "Publisher/Shared/SharedMovieclipWriter.h"

#include "DOM/FrameElement/ISymbolInstance.h"

namespace sc {
	namespace Adobe {

		Result FrameGenerator::InitializeTweenerService() {
			Result res = FCM_SUCCESS;

			if (!TweenerService)
			{
				FCM::AutoPtr<FCM::IFCMUnknown> unk;
				res = m_callback->GetService(TWEENER_SERVICE, unk.m_Ptr);
				TweenerService = unk;
				if (!TweenerService) {
					return FCM_SERVICE_NOT_FOUND;
				}
			}

			return res;
		}

		Result FrameGenerator::GetDisplayInstanceData(
			DOM::FrameElement::IFrameDisplayElement* element,
			U_Int16& identifer,
			U_Int8& blending,
			std::string& name
		) {
			// Trying to get frame element type
			AutoPtr<DOM::FrameElement::IInstance> libraryItemElement = element;

			// If frame element is Symbol or Bitmap
			if (libraryItemElement) {
				DOM::PILibraryItem libraryItem;
				libraryItemElement->GetLibraryItem(libraryItem);

				StringRep16 libraryItemName;
				libraryItem->GetName(&libraryItemName);

				std::string libraryItemName_str = Utils::ToString(libraryItemName, m_callback);

				m_resources->GetIdentifer(libraryItemName_str, identifer);

				// If Item not exported yet
				if (identifer == UINT16_MAX) {
					identifer = m_resources->AddLibraryItem(libraryItem);
				}
			}

			return FCM_SUCCESS;
		}

		Result FrameGenerator::GenerateTweenFrames(
			SharedMovieclipWriter* writer,
			DOM::ITween* tween,
			DOM::FrameElement::IFrameDisplayElement* element,
			U_Int32 duration,
			U_Int32& frameOffset
		) {
			Result res;
			FCM::AutoPtr<FCM::IFCMUnknown> unknownTweener;

			// Tweeners
			bool hasGeometricTween = false;
			AutoPtr<IGeometricTweener> matrixTweener = nullptr;

			// Tween guids dict
			PIFCMDictionary tweenerDict;
			res = tween->GetTweenedProperties(tweenerDict);
			FCM_CHECK;

			// Tweener guids
			FCMGUID geometricGuid;
			hasGeometricTween = Utils::ReadGUID(tweenerDict, kDOMGeometricProperty, geometricGuid);

			//Debug stuff
			debugLog("HasGeometricTransform: %d", hasGeometricTween);

			// Matrix tweener initialize
			if (hasGeometricTween) {
				res = TweenerService->GetTweener(geometricGuid, nullptr, unknownTweener.m_Ptr);
				matrixTweener = unknownTweener;
				unknownTweener.m_Ptr = nullptr;
				FCM_CHECK;
			}

			U_Int16 instanceIdentifer;
			U_Int8 instanceBlending;
			std::string instanceName;
			GetDisplayInstanceData(element, instanceIdentifer, instanceBlending, instanceName);

			for (U_Int32 durationIndex = 0; duration > durationIndex; durationIndex++) {
				DOM::Utils::MATRIX2D* transformMatrix = nullptr;
				DOM::Utils::COLOR_MATRIX* colorMatrix = nullptr;

				if (hasGeometricTween) {
					transformMatrix = new DOM::Utils::MATRIX2D();
					matrixTweener->GetGeometricTransform(tween, durationIndex, *transformMatrix);
				}

				writer->AddFrameElement(
					frameOffset,
					instanceIdentifer,
					instanceBlending,
					instanceName,
					transformMatrix,
					colorMatrix
				);

				frameOffset++;

				if (transformMatrix) {
					delete transformMatrix;
				}
				if (colorMatrix) {
					delete colorMatrix;
				}
			}

			return res;
		}

		Result FrameGenerator::Generate(SharedMovieclipWriter* writer, DOM::ITimeline* timeline) {
			Result res;

			res = InitializeTweenerService();
			if (FCM_FAILURE_CODE(res)) {
				console.log("Failed to initialize Tweener Service");
				return res;
			}

			// Timeline last frame
			U_Int32 framesCount = 0;
			timeline->GetMaxFrameCount(framesCount);

			debugLog("Symbol duration: %d", framesCount);

			//Initialize frames
			writer->InitTimeline(framesCount);

			// All layers in symbol
			FCM::FCMListPtr layers;
			res = timeline->GetLayers(layers.m_Ptr);
			FCM_CHECK;

			// Count of layers in symbol
			FCM::U_Int32 layerCount = 0;
			res = layers->Count(layerCount);
			FCM_CHECK;

			for (uint32_t layerIndex = 0; layerCount > layerIndex; layerIndex++) {
				AutoPtr<DOM::ILayer2> layer = layers[layerIndex];

				StringRep16 layerName;
				layer->GetName(&layerName);

				debugLog("|------------------------------ %s -------------------------------|", Utils::ToString(layerName, m_callback).c_str());

				AutoPtr<IFCMUnknown> unknownLayer;
				layer->GetLayerType(unknownLayer.m_Ptr);
				AutoPtr<DOM::Layer::ILayerNormal> normalLayer = unknownLayer;

				// Current frame position on timeline
				U_Int32 frameOffset = 0;

				// If layer is normal
				if (normalLayer) {
					// Layer max frame count
					uint32_t layerDuration;
					normalLayer->GetTotalDuration(layerDuration);

					FCM::FCMListPtr keyframes;
					normalLayer->GetKeyFrames(keyframes.m_Ptr);

					uint32_t keyframesCount = 0;
					keyframes->Count(keyframesCount);

					debugLog("Layer duration: %d, keyframes: %d", layerDuration, keyframesCount);

					// Iterate through layer keys
					for (uint32_t keyframeIndex = 0; keyframesCount > keyframeIndex; keyframeIndex++) {
						AutoPtr<DOM::IFrame> keyframe = keyframes[keyframeIndex];

						uint32_t keyframeDuration = 0;
						keyframe->GetDuration(keyframeDuration);

						AutoPtr<DOM::ITween> tween;
						keyframe->GetTween(tween.m_Ptr);

						FCM::FCMListPtr frameElements;
						keyframe->GetFrameElements(frameElements.m_Ptr);

						U_Int32 frameElementsCount = 0;
						frameElements->Count(frameElementsCount);

						debugLog("Keyframe %d. Duration: %d. Elements: %d", keyframeIndex, keyframeDuration, frameElementsCount);

						// if keyframe tweened
						if (tween && frameElementsCount == 1) {
							debugLog("Keyframe %d has tween", keyframeIndex);

							AutoPtr<DOM::FrameElement::IFrameDisplayElement> tweenedElement = frameElements[0];

							GenerateTweenFrames(writer, tween, tweenedElement, keyframeDuration, frameOffset);
						}
						else // Process as regular keyframe
						{
							for (U_Int32 frameElementIndex = 0; frameElementsCount > frameElementIndex; frameElementIndex++) {
								AutoPtr<DOM::FrameElement::IFrameDisplayElement> frameElement = frameElements[frameElementIndex];

								U_Int16 instanceIdentifer;
								U_Int8 instanceBlending;
								std::string instanceName;

								GetDisplayInstanceData(frameElement, instanceIdentifer, instanceBlending, instanceName);

								// Frames stuff
								DOM::Utils::MATRIX2D transformMatrix;
								frameElement->GetMatrix(transformMatrix);

								DOM::Utils::COLOR_MATRIX* colorMatrix = nullptr;
								AutoPtr<DOM::FrameElement::ISymbolInstance> symbolInstance = frameElement;

								if (symbolInstance) {
									colorMatrix = new DOM::Utils::COLOR_MATRIX();

									symbolInstance->GetColorMatrix(*colorMatrix);
								}

								// Iterate through frame duration
								for (U_Int32 frameOffsetIndex = 0; keyframeDuration > frameOffsetIndex; frameOffsetIndex++) {
									writer->AddFrameElement(
										frameOffset,
										instanceIdentifer,
										instanceBlending,
										instanceName,
										&transformMatrix,
										colorMatrix
									);

									frameOffset++;
								}
							}
						}
					}
				}
			};

			return res;
		}

	}
}