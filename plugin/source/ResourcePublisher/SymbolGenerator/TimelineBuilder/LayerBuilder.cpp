#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/LayerBuilder.h"
#include "ResourcePublisher/ResourcePublisher.h"
#include "ResourcePublisher/SymbolGenerator/MovieClipGenerator.h"

namespace sc {
	namespace Adobe {
		void LayerBuilder::UpdateFrame(SymbolContext& symbol) {
			FCM::AutoPtr<DOM::IFrame> frame = m_keyframes[m_keyframeIndex];
			m_frameBuilder.Update(symbol, frame);
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
				nullptr,
				nullptr
			);
		}

		LayerBuilder::LayerBuilder(
			FCM::AutoPtr<DOM::Layer::ILayerNormal> layer,
			ResourcePublisher& resources,
			SymbolContext& symbol
		) : m_symbol(symbol), m_layer(layer), m_resources(resources), m_frameBuilder(resources)
		{
			m_layer->GetTotalDuration(m_duration);
			layer->GetKeyFrames(m_keyframes.m_Ptr);
			m_keyframes->Count(m_keyframeCount);

			UpdateFrame(m_symbol);

			FCM::AutoPtr<DOM::Layer::ILayerMask> maskLayer = m_layer;
			if (maskLayer) {
				FCM::FCMListPtr maskedLayers;
				maskLayer->GetChildren(maskedLayers.m_Ptr);

				MovieClipGeneator::GetLayerBuilder(maskedLayers, resources, m_symbol, m_maskedLayers);
			}
		}

		void LayerBuilder::next() {
			m_frameBuilder.next();
			m_position++;

			// Applying next frame for masked layers
			for (LayerBuilder& layer : m_maskedLayers) {
				if (layer) {
					layer.next();
				}
			}

			// if current frame not valid anymore but layer is still has more keyframes to reading
			if (m_keyframeCount != 0) {
				if (!m_frameBuilder && m_duration > m_position) {
					m_keyframeIndex++;
					UpdateFrame(m_symbol);
				}
			}
		}

		void LayerBuilder::operator()(SharedMovieclipWriter& writer) {
			if (m_frameBuilder.empty()) {
				for (LayerBuilder& layer : m_maskedLayers) {
					if (layer) {
						layer.next();
					}
				}
				return;
			};

			{
				// Begin Masked Layers Building if masked
				bool has_masked = m_maskedLayers.size() != 0;
				if (has_masked) {
					AddModifier(writer, MaskedLayerState::MASK_LAYER);
				}

				// Realease Frames
				m_frameBuilder(m_symbol, writer);

				// Build Masked Layers
				if (has_masked) {
					AddModifier(writer, MaskedLayerState::MASKED_LAYERS);

					size_t layer_index = m_maskedLayers.size();
					for (size_t i = 0; m_maskedLayers.size() > i; i++) {
						size_t last_layer_index = layer_index;
						size_t current_layer_index = --layer_index;

						if (m_maskedLayers[current_layer_index])
						{
							LayerBuilder::ProcessLayerFrame(
								m_maskedLayers, writer,
								current_layer_index, last_layer_index,
								i == 0, current_layer_index == 0
							);
						}
					}

					AddModifier(writer, MaskedLayerState::MASKED_LAYERS_END);
				}
			}
		}

		bool LayerBuilder::shouldReleaseFilledElements(const LayerBuilder& next_layer)
		{
			if (m_frameBuilder.duration() != next_layer.frame().duration()) return true;

			if (next_layer.frame().last_element() != FrameBuilder::LastElementType::FilledElement) return true;

			return false;
		}

		void LayerBuilder::releaseFilledElements()
		{
			m_frameBuilder.releaseFilledElements(m_symbol);
		}

		void LayerBuilder::ProcessLayerFrame(
			std::vector<LayerBuilder>& layers, SharedMovieclipWriter& writer,
			size_t layer_index, size_t last_layer_index,
			bool is_begin, bool is_end
		)
		{
			LayerBuilder& layer = layers[layer_index];
			if (layer) {
				// No need to do something in first layer
				if (!is_begin)
				{
					LayerBuilder& last_layer = layers[last_layer_index];

					if (!last_layer.frame().filledElements().empty())
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

				if (is_end && !layer.frame().filledElements().empty())
				{
					layer.releaseFilledElements();
				}

				// FUCK THIS
				// IM TIRED
				// HEEEELP MEEE

				layer(writer);
			}
		}

		void LayerBuilder::ProcessLayers(SymbolContext& symbol, std::vector<LayerBuilder>& layers, SharedMovieclipWriter& writer, uint32_t range)
		{
			PluginContext& context = PluginContext::Instance();
			StatusComponent* status = context.window()->CreateStatusBar(
				context.locale.GetString("TID_BAR_LABEL_LIBRARY_ITEMS"),
				symbol.name, range
			);

			for (uint32_t t = 0; range > t; t++) {
				// First iteration to preprocess filled elements

				size_t layer_index = layers.size();
				for (size_t i = 0; layers.size() > i; i++) {
					size_t last_layer_index = layer_index;
					size_t current_layer_index = --layer_index;

					if (layers[current_layer_index])
					{
						LayerBuilder::ProcessLayerFrame(
							layers, writer,
							current_layer_index, last_layer_index,
							i == 0, current_layer_index == 0
						);
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

				writer.next();
				status->SetProgress(t);
			}

			context.window()->DestroyStatusBar(status);
		}
	}
}