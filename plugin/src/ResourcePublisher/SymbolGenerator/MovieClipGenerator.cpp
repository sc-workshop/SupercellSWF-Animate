#include "ResourcePublisher/SymbolGenerator/MovieClipGenerator.h"
#include "ResourcePublisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
		void MovieClipGeneator::GetLayerBuilder(FCM::FCMListPtr& layers, ResourcePublisher& resources, SymbolContext& symbol, vector<LayerBuilder>& result) {
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

					MovieClipGeneator::GetLayerBuilder(folderLayers, resources, symbol, result);
					continue;
				}
				else if (guideLayer) {
					FCM::FCMListPtr guideChildren;
					guideLayer->GetChildren(guideChildren.m_Ptr);

					MovieClipGeneator::GetLayerBuilder(guideChildren, resources, symbol, result);
					continue;
				}
				else if (normalLayer) {
					result.push_back(
						LayerBuilder(normalLayer, resources, symbol)
					);
				}
			}
		};

		void MovieClipGeneator::Generate(Context& context, pSharedMovieclipWriter writer, SymbolContext& symbol, FCM::AutoPtr<DOM::ITimeline1> timeline) {
			uint32_t duration = 0;
			timeline->GetMaxFrameCount(duration);

			writer->Init(context, symbol, duration);

			FCM::FCMListPtr layers;
			timeline->GetLayers(layers.m_Ptr);

			vector<LayerBuilder> layerBuilders;
			MovieClipGeneator::GetLayerBuilder(layers, m_resources, symbol, layerBuilders);

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