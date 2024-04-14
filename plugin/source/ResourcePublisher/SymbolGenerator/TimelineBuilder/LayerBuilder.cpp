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
			for (LayerBuilder& layer : m_maskedLayers) {
				if (layer) {
					layer.next();
				}
			}

			// if frame not valid anymore but layer is still valid for frame reading
			if (m_keyframeCount != 0) {
				if (!m_frameBuilder && m_duration > m_position) {
					m_keyframeIndex++;
					UpdateFrame(m_symbol);
				}
			}
		}

		void LayerBuilder::operator()(SymbolContext& symbol, SharedMovieclipWriter& writer) {
			if (m_frameBuilder.empty()) {
				for (LayerBuilder& layer : m_maskedLayers) {
					if (layer) {
						layer.next();
					}
				}
				return;
			};

			{
				// Begin Masked Layers Building
				bool has_masked = m_maskedLayers.size() != 0;
				if (has_masked) {
					AddModifier(writer, MaskedLayerState::MASK_LAYER);
				}

				// Build Mask Layer
				m_frameBuilder(symbol, writer);

				// Build Masked Layers
				if (has_masked) {
					AddModifier(writer, MaskedLayerState::MASKED_LAYERS);

					for (LayerBuilder& layer : m_maskedLayers) {
						if (layer) {
							layer(symbol, writer);
						}
					}

					AddModifier(writer, MaskedLayerState::MASKED_LAYERS_END);
				}
			}
		}
	}
}