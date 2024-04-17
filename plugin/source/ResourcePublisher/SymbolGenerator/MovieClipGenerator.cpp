#include "ResourcePublisher/SymbolGenerator/MovieClipGenerator.h"
#include "ResourcePublisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
		void MovieClipGeneator::GetLayerBuilder(FCM::FCMListPtr& layers, ResourcePublisher& resources, SymbolContext& symbol, std::vector<LayerBuilder>& result) {
			uint32_t layerCount = 0;
			layers->Count(layerCount);

			for (uint32_t i = 0; layerCount > i; i++) {
				FCM::AutoPtr<DOM::ILayer2> layer = layers[i];
				if (!layer) {
					continue;
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
					result.emplace_back(normalLayer, resources, symbol);
				}
			}
		};

		void MovieClipGeneator::Generate(SharedMovieclipWriter& writer, SymbolContext& symbol, FCM::AutoPtr<DOM::ITimeline1> timeline) {
			PluginContext& context = PluginContext::Instance();
			PluginSessionConfig& config = PluginSessionConfig::Instance();

			uint32_t duration = 0;
			timeline->GetMaxFrameCount(duration);

			SliceScalingData slice_scaling = SliceScalingData(timeline);
			if (slice_scaling.IsEnabled())
			{
				if (duration == 1)
				{
					symbol.slice_scaling = slice_scaling;
				}
				else
				{
					context.Trace(
						context.locale.GetString("TID_9SLICE_FRAME_RESTRICTION", symbol.name.c_str())
					);
				}
			}

			FCM::Double fps;
			config.document->GetFrameRate(fps);

			writer.InitializeTimeline((double)fps, duration);

			FCM::FCMListPtr layersList;
			timeline->GetLayers(layersList.m_Ptr);

			std::vector<LayerBuilder> layers;
			MovieClipGeneator::GetLayerBuilder(layersList, m_resources, symbol, layers);

			LayerBuilder::ProcessLayers(symbol, layers, writer, duration);
		}
	}
}