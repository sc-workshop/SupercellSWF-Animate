#include "Publisher/ShapeGenerator.h"
#include "Publisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
		void ShapeGenerator::InitializeService() {
			if (!BitmapExportService)
			{
				FCM::AutoPtr<FCM::IFCMUnknown> unk;
				m_callback->GetService(DOM::FLA_BITMAP_SERVICE, unk.m_Ptr);

				BitmapExportService = unk;
				if (!BitmapExportService) {
					throw exception("Failed to initialize BitmapGeneratorService");
				}
			}
		};

		
		void ShapeGenerator::GenerateLayerShapes(
			pSharedShapeWriter writer,
			AutoPtr<DOM::Layer::ILayerNormal> layer
		) {
			FCM::FCMListPtr keyframes;
			layer->GetKeyFrames(keyframes.m_Ptr);

			uint32_t keyframesCount = 0;
			keyframes->Count(keyframesCount);

			AutoPtr<DOM::IFrame> keyframe = keyframes[0];

			FCM::FCMListPtr frameElements;
			keyframe->GetFrameElements(frameElements.m_Ptr);

			uint32_t frameElementsCount = 0;
			frameElements->Count(frameElementsCount);

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
							GetImage(media, bitmapImage);

							writer->AddGraphic(bitmapImage, transformMatrix);
						}
						else {
							throw exception("Failed to get media type");
						}
						
					}
					else {
						throw exception("Failed to get media type");
					}
				}
				else {
					throw exception("Failed to get FrameElement type");
				}

			}
		}

		void ShapeGenerator::GenerateLayer(
			pSharedShapeWriter writer,
			AutoPtr<DOM::ILayer2> layer
		) {
			AutoPtr<IFCMUnknown> unknownLayer;
			layer->GetLayerType(unknownLayer.m_Ptr);

			AutoPtr<DOM::Layer::ILayerNormal> normalLayer = unknownLayer;
			AutoPtr<DOM::Layer::ILayerMask> maskLayer = unknownLayer;
			AutoPtr<DOM::Layer::ILayerGuide> guideLayer = unknownLayer;
			AutoPtr<DOM::Layer::ILayerFolder> folderLayer = unknownLayer;

			if (folderLayer) {
				FCM::FCMListPtr folderLayers;
				folderLayer->GetChildren(folderLayers.m_Ptr);
				GenerateLayerList(writer, folderLayers);
				return;
			}
			else if (guideLayer) {
				return;
			}
			else if (maskLayer) {
				// TODO: masks ?

				GenerateLayer(writer, normalLayer);

				FCM::FCMListPtr maskedLayers;
				maskLayer->GetChildren(maskedLayers.m_Ptr);

				GenerateLayerList(writer, maskedLayers);
				return;
			}
			else if (normalLayer) {
				GenerateLayerShapes(writer, normalLayer);
				return;
			}

			throw exception("Failed to get layer type");
		}

		void ShapeGenerator::GenerateLayerList(
			pSharedShapeWriter writer,
			FCMListPtr layers
		) {
			U_Int32 layerCount = 0;
			layers->Count(layerCount);

			uint32_t i = layerCount;
			for (U_Int32 layerIndex = 0; layerCount > layerIndex; layerIndex++) {
				AutoPtr<DOM::ILayer2> layer = layers[--i];

				GenerateLayer(writer, layer);
			};
		}

		void ShapeGenerator::GetImage(AutoPtr<DOM::LibraryItem::IMediaItem>& media, cv::Mat& image) {
			Result res = BitmapExportService->ExportToFile(
				media,
				Utils::ToString16(tempFile.string(), m_callback),
				100
			);
			if (FCM_FAILURE_CODE(res)) {
				throw exception("Failed to export image");
			}

			image = cv::imread(tempFile.string(), cv::IMREAD_UNCHANGED);
		}
		

		void ShapeGenerator::Generate(pSharedShapeWriter writer, DOM::ITimeline* timeline) {
			InitializeService();

			FCM::FCMListPtr layers;
			timeline->GetLayers(layers.m_Ptr);

			GenerateLayerList(writer, layers);
		}

		// Validate stuff

		bool ShapeGenerator::ValidateLayerItems(
			AutoPtr<DOM::Layer::ILayerNormal> layer
		) {
			FCM::FCMListPtr keyframes;
			layer->GetKeyFrames(keyframes.m_Ptr);

			uint32_t keyframesCount = 0;
			keyframes->Count(keyframesCount);

			if (keyframesCount != 1) {
				return false;
			}

			AutoPtr<DOM::IFrame> keyframe = keyframes[0];

			FCM::FCMListPtr frameElements;
			keyframe->GetFrameElements(frameElements.m_Ptr);

			uint32_t frameElementsCount = 0;
			frameElements->Count(frameElementsCount);

			for (uint32_t i = 0; frameElementsCount > i; i++) {
				AutoPtr<DOM::FrameElement::IInstance> instance = frameElements[i];

				if (!instance) {
					return false;
				}

				AutoPtr<DOM::ILibraryItem> item;
				instance->GetLibraryItem(item.m_Ptr);

				AutoPtr<DOM::LibraryItem::IMediaItem> media = item;

				if (!media) {
					return false;
				}
			}

			return true;
		}

		bool ShapeGenerator::ValidateLayer(
			AutoPtr<DOM::ILayer2> layer
		) {
			AutoPtr<IFCMUnknown> unknownLayer;
			layer->GetLayerType(unknownLayer.m_Ptr);

			AutoPtr<DOM::Layer::ILayerNormal> normalLayer = unknownLayer;
			AutoPtr<DOM::Layer::ILayerMask> maskLayer = unknownLayer;
			AutoPtr<DOM::Layer::ILayerGuide> guideLayer = unknownLayer;
			AutoPtr<DOM::Layer::ILayerFolder> folderLayer = unknownLayer;

			if (folderLayer) {
				FCM::FCMListPtr folderLayers;
				folderLayer->GetChildren(folderLayers.m_Ptr);

				return ValidateLayerList(folderLayers);
			}
			else if (guideLayer) {
				return true;
			}
			else if (maskLayer) {
				return ValidateLayer(normalLayer);

				FCM::FCMListPtr maskedLayers;
				maskLayer->GetChildren(maskedLayers.m_Ptr);

				return ValidateLayerList(maskedLayers);
			}
			else if (normalLayer) {
				return ValidateLayerItems(normalLayer);
			}

			return false;
		}

		bool ShapeGenerator::ValidateLayerList(
			FCMListPtr layers
		) {

			U_Int32 layerCount = 0;
			layers->Count(layerCount);

			for (uint32_t i = 0; layerCount > i; i++) {
				AutoPtr<DOM::ILayer2> layer = layers[i];

				if (!ValidateLayer(layer)) {
					return false;
				}
			};

			return true;
		}

		bool ShapeGenerator::Validate(DOM::ITimeline* timeline) {

			FCM::FCMListPtr layers;
			timeline->GetLayers(layers.m_Ptr);

			uint32_t timelineDuration;
			timeline->GetMaxFrameCount(timelineDuration);

			if (timelineDuration > 1) {
				return false;
			}

			return ValidateLayerList(layers);
		}
	}
}