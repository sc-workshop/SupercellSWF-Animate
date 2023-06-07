#include "Publisher/TimelineBuilder/Builder.h"

namespace sc {
	namespace Adobe {
		void TimelineBuilder::GetLayerBuilder(FCMListPtr& layers, ResourcePublisher& resources, vector<LayerBuilder>& result) {
			uint32_t layerCount = 0;
			layers->Count(layerCount);

			for (uint32_t i = 0; layerCount > i; i++) {
				AutoPtr<DOM::ILayer2> layer = layers[i];
				if (!layer) {
					throw exception("Failed to get layer");
				}

				AutoPtr<IFCMUnknown> unknownLayer;
				layer->GetLayerType(unknownLayer.m_Ptr);

				AutoPtr<DOM::Layer::ILayerNormal> normalLayer = unknownLayer;
				AutoPtr<DOM::Layer::ILayerGuide> guideLayer = unknownLayer;
				AutoPtr<DOM::Layer::ILayerFolder> folderLayer = unknownLayer;

				if (folderLayer) {
					FCM::FCMListPtr folderLayers;
					folderLayer->GetChildren(folderLayers.m_Ptr);

					TimelineBuilder::GetLayerBuilder(folderLayers, resources, result);
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

		void TimelineBuilder::Generate(pSharedMovieclipWriter writer, AutoPtr<DOM::ITimeline> timeline) {
			uint32_t duration = 0;
			timeline->GetMaxFrameCount(duration);

			debugLog("Symbol duration: %d", duration);

			writer->InitTimeline(duration);

			FCM::FCMListPtr layers;
			timeline->GetLayers(layers.m_Ptr);

			vector<LayerBuilder> layerBuilders;
			TimelineBuilder::GetLayerBuilder(layers, m_resources, layerBuilders);

			for (uint32_t t = 0; duration > t; t++) {
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