#include "Publisher/TimelineBuilder/Builder.h"
#include "Publisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
		void TimelineBuilder::GetLayerBuilder(FCM::FCMListPtr& layers, ResourcePublisher& resources, vector<LayerBuilder>& result) {
			uint32_t layerCount = 0;
			layers->Count(layerCount);

			for (uint32_t i = 0; layerCount > i; i++) {
				FCM::AutoPtr<DOM::ILayer2> layer = layers[i];
				if (!layer) {
					throw exception("Failed to get layer");
				}

				FCM::AutoPtr<FCM::IFCMUnknown> unknownLayer;
				layer->GetLayerType(unknownLayer.m_Ptr);

				FCM::AutoPtr<DOM::Layer::ILayerNormal> normalLayer = unknownLayer;
				FCM::AutoPtr<DOM::Layer::ILayerGuide> guideLayer = unknownLayer;
				FCM::AutoPtr<DOM::Layer::ILayerFolder> folderLayer = unknownLayer;

				if (folderLayer) {
					FCM::FCMListPtr folderLayers;
					folderLayer->GetChildren(folderLayers.m_Ptr);

					TimelineBuilder::GetLayerBuilder(folderLayers, resources, result);
					continue;
				}
				else if (guideLayer) {
					continue;
				}
				else if (normalLayer) {
					result.push_back(
						LayerBuilder(normalLayer, resources)
					);
				}
			}
		};

		void TimelineBuilder::Generate(pSharedMovieclipWriter writer, FCM::AutoPtr<DOM::ITimeline> timeline) {
			uint32_t duration = 0;
			timeline->GetMaxFrameCount(duration);

			writer->InitTimeline(duration);

			FCM::FCMListPtr layers;
			timeline->GetLayers(layers.m_Ptr);

			vector<LayerBuilder> layerBuilders;
			TimelineBuilder::GetLayerBuilder(layers, m_resources, layerBuilders);

			for (uint32_t t = 0; duration > t; t++){
				//m_resources.context.window->frame->SetProgress((uint8_t)(duration / t * 100));

				uint32_t i = (uint32_t)layerBuilders.size();
				for (uint32_t layerIndex = 0; layerBuilders.size() > layerIndex; layerIndex++) {
					LayerBuilder& layer = layerBuilders[--i];
					if (layer) {
						layer(writer);
						layer.next();
					}
				}
				writer->next();
			}
		}
	}
}