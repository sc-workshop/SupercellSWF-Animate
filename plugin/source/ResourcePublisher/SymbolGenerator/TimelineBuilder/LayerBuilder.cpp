#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/LayerBuilder.h"
#include "ResourcePublisher/ResourcePublisher.h"
#include "ResourcePublisher/SymbolGenerator/MovieClipGenerator.h"

namespace sc {
	namespace Adobe {
		void LayerBuilder::UpdateFrame(SymbolContext& symbol) {
			FCM::AutoPtr<DOM::IFrame> frame = m_keyframes[m_keyframeIndex];
			frameBuilder.Update(symbol, frame);
		}

		void LayerBuilder::AddModifier(
			SharedMovieclipWriter& writer,
			MaskedLayerState type
		) {
			uint16_t identifer = m_resources.GetIdentifer(type);

			if (identifer == UINT16_MAX) {
				identifer = m_resources.AddModifier(type);
			}

			writer.AddFrameElement(
				identifer,
				FCM::BlendMode::NORMAL_BLEND_MODE,
				u"",
				std::nullopt,
				std::nullopt
			);
		}

		LayerBuilder::LayerBuilder(
			FCM::AutoPtr<DOM::Layer::ILayerNormal> layer,
			uint32_t duration,
			ResourcePublisher& resources,
			SymbolContext& symbol
		) : m_symbol(symbol), m_duration(duration), m_layer(layer), m_resources(resources), frameBuilder(resources)
		{
			layer->GetKeyFrames(m_keyframes.m_Ptr);
			m_keyframes->Count(m_keyframeCount);

			UpdateFrame(m_symbol);

			FCM::AutoPtr<DOM::Layer::ILayerMask> maskLayer = m_layer;
			if (maskLayer) {
				FCM::FCMListPtr layers;
				maskLayer->GetChildren(layers.m_Ptr);

				m_mask_layer = true;
				MovieClipGeneator::GetLayerBuilder(layers, resources, m_symbol, maskedLayers);
			}
		}

		void LayerBuilder::next() {
			frameBuilder.next();
			m_position++;

			// if current frame not valid anymore but layer is still has more keyframes to reading
			if (m_keyframeCount != 0) {
				if (!frameBuilder && m_duration > m_position) {
					m_keyframeIndex++;
					UpdateFrame(m_symbol);
				}
			}
		}

		void LayerBuilder::operator()(SharedMovieclipWriter& writer) {
			frameBuilder(m_symbol, writer);
		}

		bool LayerBuilder::shouldReleaseFilledElements(const LayerBuilder& next_layer)
		{
			if (frameBuilder.duration() != next_layer.frameBuilder.duration()) return true;

			if (frameBuilder.position() != next_layer.frameBuilder.position()) return true;

			if (next_layer.frameBuilder.last_element() != FrameBuilder::LastElementType::FilledElement) return true;

			return false;
		}

		void LayerBuilder::releaseFilledElements()
		{
			frameBuilder.releaseFilledElements(m_symbol, std::u16string(u""));
		}

		void LayerBuilder::ProcessLayerFrame(
			std::vector<LayerBuilder>& layers, SharedMovieclipWriter& writer,
			size_t layer_index, size_t last_layer_index,
			bool is_begin, bool is_end
		)
		{
			LayerBuilder& layer = layers[layer_index];
			if (layer) {
				if (layer.maskLayer() && layer.canReleaseFilledElements())
				{
					layer.releaseFilledElements();

					// Just skip all of checks. There is no need for them just for mask layer.
					goto FINALIZE_LAYER;
				}

				// No need to do something in first layer
				if (!is_begin)
				{
					LayerBuilder& last_layer = layers[last_layer_index];

					if (!last_layer.frameBuilder.filledElements().empty())
					{
						layer.inheritFilledElements(last_layer);

						if (last_layer.shouldReleaseFilledElements(layer)) {
							layer.releaseFilledElements();
						}
						else
						{
							if (!is_end)
							{
								return;
							}
						}
					}
				}

				if (is_end && layer.canReleaseFilledElements())
				{
					layer.releaseFilledElements();
				}

			FINALIZE_LAYER:
				layer.frameBuilder.ApplyName(writer);
				layer(writer);
			}
		}

		void LayerBuilder::ProcessLayers(SymbolContext& context, std::vector<LayerBuilder>& layers, SharedMovieclipWriter& writer)
		{
			// First iteration to preprocess filled elements
			size_t layer_index = layers.size();
			for (size_t i = 0; layers.size() > i; i++) {
				size_t last_layer_index = layer_index;
				size_t current_layer_index = --layer_index;

				LayerBuilder& current_layer = layers[current_layer_index];
				if (current_layer)
				{
					bool is_masked_frame = current_layer.maskLayer() && current_layer.frameBuilder.flushMask();
					if (is_masked_frame) {
						current_layer.AddModifier(writer, MaskedLayerState::MASK_LAYER);
					}

					LayerBuilder::ProcessLayerFrame(
						layers, writer,
						current_layer_index, last_layer_index,
						i == 0, current_layer_index == 0
					);

					if (is_masked_frame)
					{
						current_layer.AddModifier(writer, MaskedLayerState::MASKED_LAYERS);
						LayerBuilder::ProcessLayers(context, current_layer.maskedLayers, writer);
						current_layer.AddModifier(writer, MaskedLayerState::MASKED_LAYERS_END);
					}
					else
					{
						LayerBuilder::ProcessLayers(context, current_layer.maskedLayers, writer);
					}
				}
			}

			// Final iteration to  change layers current positions
			for (LayerBuilder& layer : layers)
			{
				if (layer)
				{
					layer.next();
				}
			}
		}

		void LayerBuilder::ProcessLayers(SymbolContext& symbol, std::vector<LayerBuilder>& layers, SharedMovieclipWriter& writer, uint32_t range)
		{
			PluginContext& context = PluginContext::Instance();
			StatusComponent* status = context.Window()->CreateStatusBarComponent(
				context.locale.GetString("TID_BAR_LABEL_LIBRARY_ITEMS"),
				symbol.name, range
			);

			for (uint32_t t = 0; range > t; t++) {
				LayerBuilder::ProcessLayers(symbol, layers, writer);
				writer.next();

				status->SetProgress(t);
			}

			context.Window()->DestroyStatusBar(status);
		}
	}
}