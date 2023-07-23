#include "Publisher/ShapeGenerator.h"
#include "Publisher/ResourcePublisher.h"

#include "DOM/FrameElement/IShape.h"

namespace sc {
	namespace Adobe {
		ShapeGenerator::ShapeGenerator(ResourcePublisher& resources) :
			m_resources(resources) {
			BitmapExportService = resources.context.getService<IBitmapExportService>(DOM::FLA_BITMAP_SERVICE);
		};

		ShapeGenerator::~ShapeGenerator() {
			if (fs::exists(tempFile)) {
				remove(tempFile);
			}
		};

		void ShapeGenerator::GenerateLayerElements(
			pSharedShapeWriter writer,
			FCM::FCMListPtr frameElements
		) {
			uint32_t frameElementsCount = 0;
			frameElements->Count(frameElementsCount);

			uint32_t frameElementIndex = frameElementsCount;
			for (uint32_t i = 0; frameElementsCount > i; i++) {
				FCM::AutoPtr<DOM::FrameElement::IFrameDisplayElement> frameElement = frameElements[--frameElementIndex];

				DOM::Utils::MATRIX2D transformMatrix;
				frameElement->GetMatrix(transformMatrix);

				FCM::AutoPtr<DOM::FrameElement::IInstance> instance = frameElement;
				FCM::AutoPtr<DOM::FrameElement::IShape> shape = frameElement;
				FCM::AutoPtr<DOM::FrameElement::IGroup> group = frameElement;

				// Bitmap
				if (instance) {
					FCM::AutoPtr<DOM::ILibraryItem> item;
					instance->GetLibraryItem(item.m_Ptr);

					FCM::StringRep16 itemNamePtr;
					item->GetName(&itemNamePtr);
					u16string itemName = (const char16_t*)itemNamePtr;
					m_resources.context.falloc->Free(itemNamePtr);

					FCM::AutoPtr<DOM::LibraryItem::IMediaItem> media = item;

					if (media)
					{
						FCM::AutoPtr<FCM::IFCMUnknown> unknownMedia;
						media->GetMediaInfo(unknownMedia.m_Ptr);

						FCM::AutoPtr<DOM::MediaInfo::IBitmapInfo> bitmap = unknownMedia;

						if (bitmap) {
							cv::Mat bitmapImage;
							if (!m_resources.GetCachedBitmap(itemName, bitmapImage)) {
								GetImage(media, bitmapImage);
								m_resources.AddCachedBitmap(itemName, bitmapImage);
							}

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

				// Fills / Stroke
				else if (shape)
				{
					FilledShape shape(m_resources.context, shape);

					writer->AddFilledShape(shape, false);
				}

				// Group of elements
				else if (group)
				{
					FCM::FCMListPtr groupElements;
					group->GetMembers(groupElements.m_Ptr);

					GenerateLayerElements(writer, groupElements);
				}
			}
		}

		void ShapeGenerator::GenerateLayerShapes(
			pSharedShapeWriter writer,
			FCM::AutoPtr<DOM::Layer::ILayerNormal> layer
		) {
			FCM::FCMListPtr keyframes;
			layer->GetKeyFrames(keyframes.m_Ptr);

			uint32_t keyframesCount = 0;
			keyframes->Count(keyframesCount);

			FCM::AutoPtr<DOM::IFrame> keyframe = keyframes[0];

			FCM::FCMListPtr frameElements;
			keyframe->GetFrameElements(frameElements.m_Ptr);

			GenerateLayerElements(writer, frameElements);
		}

		void ShapeGenerator::GenerateLayer(
			pSharedShapeWriter writer,
			FCM::AutoPtr<DOM::ILayer2> layer
		) {
			FCM::AutoPtr<FCM::IFCMUnknown> unknownLayer;
			layer->GetLayerType(unknownLayer.m_Ptr);

			FCM::AutoPtr<DOM::Layer::ILayerNormal> normalLayer = unknownLayer;
			FCM::AutoPtr<DOM::Layer::ILayerMask> maskLayer = unknownLayer;
			FCM::AutoPtr<DOM::Layer::ILayerGuide> guideLayer = unknownLayer;
			FCM::AutoPtr<DOM::Layer::ILayerFolder> folderLayer = unknownLayer;

			if (folderLayer) {
				FCM::FCMListPtr folderLayers;
				folderLayer->GetChildren(folderLayers.m_Ptr);
				GenerateLayerList(writer, folderLayers);
				return;
			}
			else if (guideLayer) {
				FCM::FCMListPtr childrens;
				guideLayer->GetChildren(childrens.m_Ptr);
				GenerateLayerList(writer, childrens);
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
			FCM::FCMListPtr layers
		) {
			uint32_t layerCount = 0;
			layers->Count(layerCount);

			uint32_t i = layerCount;
			for (uint32_t layerIndex = 0; layerCount > layerIndex; layerIndex++) {
				FCM::AutoPtr<DOM::ILayer2> layer = layers[--i];

				GenerateLayer(writer, layer);
			};
		}

		void ShapeGenerator::GetImage(FCM::AutoPtr<DOM::LibraryItem::IMediaItem>& media, cv::Mat& image) {
			FCM::Result res = BitmapExportService->ExportToFile(
				media,
				(FCM::CStringRep16)tempFile.u16string().c_str(),
				100
			);
			if (FCM_FAILURE_CODE(res)) {
				throw exception("Failed to export image");
			}

			image = cv::imread(tempFile.string(), cv::IMREAD_UNCHANGED);
		}

		void ShapeGenerator::Generate(pSharedShapeWriter writer, DOM::ITimeline* timeline) {
			FCM::FCMListPtr layers;
			timeline->GetLayers(layers.m_Ptr);

			GenerateLayerList(writer, layers);
		}

		// Validate stuff

		bool ShapeGenerator::ValidateLayerFrameElements(FCM::FCMListPtr frameElements) {
			uint32_t frameElementsCount = 0;
			frameElements->Count(frameElementsCount);

			for (uint32_t i = 0; frameElementsCount > i; i++) {
				FCM::AutoPtr<DOM::FrameElement::IInstance> instance = frameElements[i];
				FCM::AutoPtr<DOM::FrameElement::IShape> shape = frameElements[i];
				FCM::AutoPtr<DOM::FrameElement::IGroup> group = frameElements[i];

				if (group) {
					FCM::FCMListPtr groupElements;
					group->GetMembers(groupElements.m_Ptr);

					if (!ValidateLayerFrameElements(groupElements)) {
						return false;
					}
				}
				else if (instance) {
					FCM::AutoPtr<DOM::ILibraryItem> item;
					instance->GetLibraryItem(item.m_Ptr);

					FCM::AutoPtr<DOM::LibraryItem::IMediaItem> media = item;

					if (!media) {
						return false;
					}
				}
				else if (shape) {
					continue;
				}
				else {
					return false;
				}
			}

			return true;
		}

		bool ShapeGenerator::ValidateLayerItems(
			FCM::AutoPtr<DOM::Layer::ILayerNormal> layer
		) {
			FCM::FCMListPtr keyframes;
			layer->GetKeyFrames(keyframes.m_Ptr);

			uint32_t keyframesCount = 0;
			keyframes->Count(keyframesCount);

			if (keyframesCount != 1) {
				return false;
			}

			FCM::AutoPtr<DOM::IFrame> keyframe = keyframes[0];

			FCM::FCMListPtr frameElements;
			keyframe->GetFrameElements(frameElements.m_Ptr);

			return ValidateLayerFrameElements(frameElements);
		}

		bool ShapeGenerator::ValidateLayer(
			FCM::AutoPtr<DOM::ILayer2> layer
		) {
			FCM::AutoPtr<FCM::IFCMUnknown> unknownLayer;
			layer->GetLayerType(unknownLayer.m_Ptr);

			FCM::AutoPtr<DOM::Layer::ILayerNormal> normalLayer = unknownLayer;
			FCM::AutoPtr<DOM::Layer::ILayerMask> maskLayer = unknownLayer;
			FCM::AutoPtr<DOM::Layer::ILayerGuide> guideLayer = unknownLayer;
			FCM::AutoPtr<DOM::Layer::ILayerFolder> folderLayer = unknownLayer;

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
			FCM::FCMListPtr layers
		) {
			uint32_t layerCount = 0;
			layers->Count(layerCount);

			for (uint32_t i = 0; layerCount > i; i++) {
				FCM::AutoPtr<DOM::ILayer2> layer = layers[i];

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