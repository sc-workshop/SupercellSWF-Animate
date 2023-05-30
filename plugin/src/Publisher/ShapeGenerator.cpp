#include "Publisher/ShapeGenerator.h"

#include <DOM/ILibraryItem.h>
#include <Utils/DOMTypes.h>
#include <DOM/FrameElement/IInstance.h>
#include <DOM/LibraryItem/IMediaItem.h>
#include <DOM/MediaInfo/IBitmapInfo.h>

namespace sc {
	namespace Adobe {
		Result ShapeGenerator::InitializeService() {
			Result res = FCM_SUCCESS;

			if (!BitmapExportService)
			{
				FCM::AutoPtr<FCM::IFCMUnknown> unk;
				res = m_callback->GetService(DOM::FLA_BITMAP_SERVICE, unk.m_Ptr);
				FCM_CHECK;

				BitmapExportService = unk;
				if (!BitmapExportService) {
					return FCM_SERVICE_NOT_FOUND;
				}
			}

			return res;
		};

		
		Result ShapeGenerator::GenerateLayerShapes(
			pSharedShapeWriter writer,
			AutoPtr<DOM::Layer::ILayerNormal> layer
		) {
			Result res = FCM_SUCCESS;

			FCM::FCMListPtr keyframes;
			res = layer->GetKeyFrames(keyframes.m_Ptr);
			FCM_CHECK;

			uint32_t keyframesCount = 0;
			res = keyframes->Count(keyframesCount);
			FCM_CHECK;

			AutoPtr<DOM::IFrame> keyframe = keyframes[0];

			FCM::FCMListPtr frameElements;
			res = keyframe->GetFrameElements(frameElements.m_Ptr);
			FCM_CHECK;

			uint32_t frameElementsCount = 0;
			res = frameElements->Count(frameElementsCount);
			FCM_CHECK;

			uint32_t frameElementIndex = frameElementsCount;
			for (uint32_t i = 0; frameElementsCount > i; i++) {
				AutoPtr<DOM::FrameElement::IFrameDisplayElement> frameElement = frameElements[--frameElementIndex];

				DOM::Utils::MATRIX2D transformMatrix;
				frameElement->GetMatrix(transformMatrix);

				//TODO: Color transform for bitmaps?

				AutoPtr<DOM::FrameElement::IInstance> instance = frameElement.m_Ptr;

				if (instance) {
					AutoPtr<DOM::ILibraryItem> item;
					instance->GetLibraryItem(item.m_Ptr);

					AutoPtr<DOM::LibraryItem::IMediaItem> media = item;

					if (media) {
						AutoPtr<IFCMUnknown> unknownMedia;
						media->GetMediaInfo(unknownMedia.m_Ptr);

						AutoPtr<DOM::MediaInfo::IBitmapInfo> bitmap = unknownMedia;

						if (bitmap) {
							cv::Mat bitmapImage;
							res = GetImage(media, bitmapImage);
							FCM_CHECK;

							writer->AddGraphic(bitmapImage, transformMatrix);
						}
						else {
							return FCM_EXPORT_FAILED;
						}
						
					}
					else {
						return FCM_EXPORT_FAILED;
					}
				}
				else {
					return FCM_EXPORT_FAILED;
				}

			}

			return res;
		}

		Result ShapeGenerator::GenerateLayer(
			pSharedShapeWriter writer,
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
				return res;
			}
			else if (maskLayer) {
				// TODO: masks ?

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

				return GenerateLayerList(writer, maskedLayers);;
			}
			else if (normalLayer) {
				return GenerateLayerShapes(writer, normalLayer);;
			}

			// Layer must pass at least one of conditions above
			return FCM_EXPORT_FAILED;
		}

		Result ShapeGenerator::GenerateLayerList(
			pSharedShapeWriter writer,
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

		Result ShapeGenerator::GetImage(AutoPtr<DOM::LibraryItem::IMediaItem>& media, cv::Mat& image) {
			Result res = BitmapExportService->ExportToFile(
				media,
				Utils::ToString16(tempFile.string(), m_callback),
				100
			);
			FCM_CHECK;

			image = cv::imread(tempFile.string(), cv::IMREAD_UNCHANGED);

			return res;
		}
		

		Result ShapeGenerator::Generate(pSharedShapeWriter writer, DOM::ITimeline* timeline) {
			Result res;

			res = InitializeService();
			FCM_CHECK;

			FCM::FCMListPtr layers;
			res = timeline->GetLayers(layers.m_Ptr);
			FCM_CHECK;

			return GenerateLayerList(writer, layers);
		}

		// Validate stuff

		Result ShapeGenerator::ValidateLayerItems(
			AutoPtr<DOM::Layer::ILayerNormal> layer,
			bool& result
		) {
			Result res = FCM_SUCCESS;

			FCM::FCMListPtr keyframes;
			res = layer->GetKeyFrames(keyframes.m_Ptr);
			FCM_CHECK;

			uint32_t keyframesCount = 0;
			res = keyframes->Count(keyframesCount);
			FCM_CHECK;

			if (keyframesCount != 1) {
				result = false;
				return res;
			}

			AutoPtr<DOM::IFrame> keyframe = keyframes[0];

			FCM::FCMListPtr frameElements;
			res = keyframe->GetFrameElements(frameElements.m_Ptr);
			FCM_CHECK;

			uint32_t frameElementsCount = 0;
			res = frameElements->Count(frameElementsCount);
			FCM_CHECK;

			for (uint32_t i = 0; frameElementsCount > i; i++) {
				AutoPtr<DOM::FrameElement::IInstance> instance = frameElements[i];

				if (!instance) {
					result = false;
					return res;
				}

				AutoPtr<DOM::ILibraryItem> item;
				instance->GetLibraryItem(item.m_Ptr);

				AutoPtr<DOM::LibraryItem::IMediaItem> media = item;

				if (!media) {
					result = false;
					return res;
				}
			}

			return res;
		}

		Result ShapeGenerator::ValidateLayer(
			AutoPtr<DOM::ILayer2> layer,
			bool& result
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
				res = ValidateLayerList(folderLayers, result);
				return res;
			}
			else if (guideLayer) {
				return res;
			}
			else if (maskLayer) {
				if (!normalLayer) {
					return FCM_EXPORT_FAILED;
				}
				else {
					res = ValidateLayer(normalLayer, result);
					FCM_CHECK;
				}

				FCM::FCMListPtr maskedLayers;
				res = maskLayer->GetChildren(maskedLayers.m_Ptr);
				FCM_CHECK;

				return ValidateLayerList(maskedLayers, result);
			}
			else if (normalLayer) {
				return ValidateLayerItems(normalLayer, result);
			}

			return FCM_EXPORT_FAILED;
		}

		Result ShapeGenerator::ValidateLayerList(
			FCMListPtr layers,
			bool& result
		) {
			Result res = FCM_SUCCESS;

			U_Int32 layerCount = 0;
			res = layers->Count(layerCount);
			FCM_CHECK;

			uint32_t i = layerCount;
			for (U_Int32 layerIndex = 0; layerCount > layerIndex; layerIndex++) {
				AutoPtr<DOM::ILayer2> layer = layers[--i];

				res = ValidateLayer(layer, result);
				FCM_CHECK;
			};

			return res;
		}

		Result ShapeGenerator::Validate(DOM::ITimeline* timeline, bool& result) {
			Result res = FCM_SUCCESS;

			FCM::FCMListPtr layers;
			res = timeline->GetLayers(layers.m_Ptr);
			FCM_CHECK;

			uint32_t timelineDuration;
			res = timeline->GetMaxFrameCount(timelineDuration);
			FCM_CHECK;

			if (timelineDuration > 1) {
				result = false;
				return FCM_SUCCESS;
			}

			return ValidateLayerList(layers, result);
		}
	}
}