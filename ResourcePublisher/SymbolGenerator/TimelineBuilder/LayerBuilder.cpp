#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/LayerBuilder.h"
#include "ResourcePublisher/ResourcePublisher.h"
#include "ResourcePublisher/SymbolGenerator/MovieClipGenerator.h"

namespace sc {
	namespace Adobe {
		void LayerBuilder::UpdateFrame() {
			FCM::AutoPtr<DOM::IFrame> frame = m_keyframes[m_keyframeIndex];
			m_frameBuilder.Update(frame);
		}

		void LayerBuilder::AddModifier(
			pSharedMovieclipWriter writer,
			sc::MovieClipModifier::Type type
		) {
			uint16_t identifer = m_resources.GetIdentifer(type);

			if (identifer == 0xFFFF) {
				identifer = m_resources.AddModifier(type);
			}

			writer->AddFrameElement(
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
			SymbolBehaviorInfo& info
		) : m_layer(layer), m_resources(resources), m_frameBuilder(resources), m_behavior(info)
		{
			m_layer->GetTotalDuration(m_duration);
			layer->GetKeyFrames(m_keyframes.m_Ptr);
			m_keyframes->Count(m_keyframeCount);

			UpdateFrame();

			FCM::AutoPtr<DOM::Layer::ILayerMask> maskLayer = m_layer;
			if (maskLayer) {
				FCM::FCMListPtr maskedLayers;
				maskLayer->GetChildren(maskedLayers.m_Ptr);

				m_behavior.hasSlice9 = false;
				TimelineBuilder::GetLayerBuilder(maskedLayers, resources, m_behavior, m_maskedLayers);
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
					UpdateFrame();
				}
			}
		}

		void LayerBuilder::operator()(pSharedMovieclipWriter writer) {
			if (m_frameBuilder.empty()) {
				for (LayerBuilder& layer : m_maskedLayers) {
					if (layer) {
						layer.next();
					}
				}
				return;
			};

			bool isMaskLayer = m_maskedLayers.size() != 0;
			if (isMaskLayer) {
				AddModifier(writer, sc::MovieClipModifier::Type::Mask);
			}

			m_frameBuilder(writer);

			if (isMaskLayer) {
				AddModifier(writer, sc::MovieClipModifier::Type::Masked);

				for (LayerBuilder& layer : m_maskedLayers) {
					if (layer) {
						layer(writer);
					}
				}

				AddModifier(writer, sc::MovieClipModifier::Type::Unmasked);
			}
		}
	}
}