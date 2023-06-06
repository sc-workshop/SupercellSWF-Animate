#include "Publisher/TimelineBuilder/FrameBuilder.h"

#include "Publisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
		void FrameBuilder::update(AutoPtr<DOM::IFrame>& frame) {
			if (!frame) {
				throw exception("Failed to get NULL frame");
			}

			// cleaning
			m_duration = 0;
			m_position = 0;
			m_name = "";
			m_elementsData.clear();
			m_matrices.clear();
			m_colors.clear();
			m_tween = nullptr;
			m_matrixTweener = nullptr;
			m_colorTweener = nullptr;

			// Frame processing

			frame->GetDuration(m_duration);

			// Frame elements processing
			FCMListPtr frameElements;
			uint32_t frameElementsCount = 0;
			frame->GetFrameElements(frameElements.m_Ptr);
			frameElements->Count(frameElementsCount);

			uint32_t i = frameElementsCount;
			for (uint32_t elementIndex = 0; frameElementsCount > elementIndex; elementIndex++) {
				AutoPtr<DOM::FrameElement::IFrameDisplayElement> frameElement = frameElements[--i];
				if (!frameElement) {
					throw exception("Failed to get frame element");
				}

				uint16_t id = 0xFFFF;
				MATRIX2D* matrix = NULL;
				COLOR_MATRIX* color = NULL;

				AutoPtr<DOM::FrameElement::IInstance> libraryElement = frameElement;
				AutoPtr<DOM::FrameElement::ISymbolInstance> symbolItem = frameElement;

				if (libraryElement) {
					DOM::PILibraryItem libraryItem;
					libraryElement->GetLibraryItem(libraryItem);

					StringRep16 itemNamePtr;
					libraryItem->GetName(&itemNamePtr);
					u16string itemName = (const char16_t*)itemNamePtr;
					m_resources.GetCallocService()->Free(itemNamePtr);

					id = m_resources.GetIdentifer(itemName);

					// If Item not exported yet
					if (id == UINT16_MAX) {
						id = m_resources.AddLibraryItem(libraryItem);
					}

					// Transform 
					matrix = new MATRIX2D();
					frameElement->GetMatrix(*matrix);

					if (symbolItem) {
						color = new COLOR_MATRIX();
						symbolItem->GetColorMatrix(*color);
					}
				}

				if (id == 0xFFFF) {
					throw exception("Failed to get frame element id");
				}

				m_elementsData.push_back({
					id,
					0,
					"" // TODO Blending / Element names
				});

				m_matrices.push_back(
					shared_ptr<MATRIX2D>(matrix)
				);

				m_colors.push_back(
					shared_ptr<COLOR_MATRIX>(color)
				);
			}

			// Tween
			frame->GetTween(m_tween.m_Ptr);
			if (m_tween) {
				PIFCMDictionary tweenerDict;
				m_tween->GetTweenedProperties(tweenerDict);

				FCMGUID matrixGuid;
				FCMGUID colorGuid;

				bool hasMatrixTweener = Utils::ReadGUID(tweenerDict, kDOMGeometricProperty, matrixGuid);
				bool hasColorTweener = Utils::ReadGUID(tweenerDict, kDOMColorProperty, colorGuid);

				AutoPtr<ITweenerService> TweenerService = m_resources.GetTweenerService();

				FCM::AutoPtr<FCM::IFCMUnknown> unknownTweener;
				if (hasMatrixTweener) {
					TweenerService->GetTweener(matrixGuid, nullptr, unknownTweener.m_Ptr);
					m_matrixTweener = unknownTweener;
					unknownTweener.m_Ptr = nullptr;
				}
				if (hasColorTweener) {
					TweenerService->GetTweener(colorGuid, nullptr, unknownTweener.m_Ptr);
					m_colorTweener = unknownTweener;
					unknownTweener.m_Ptr = nullptr;
				}
			}
		}

		void FrameBuilder::operator()(pSharedMovieclipWriter writer) {
			uint32_t i = (uint32_t)m_elementsData.size();
			for (uint32_t elementIndex = 0; m_elementsData.size() > elementIndex; elementIndex++) {
				i--;

				DOM::Utils::MATRIX2D* matrix = nullptr;
				DOM::Utils::COLOR_MATRIX* color = nullptr;

				if (m_matrices[i]) {
					matrix = new DOM::Utils::MATRIX2D(*(m_matrices[i].get()));

					if (m_matrixTweener) {
						DOM::Utils::MATRIX2D baseMatrix(*matrix);
						DOM::Utils::MATRIX2D transformMatrix;
						m_matrixTweener->GetGeometricTransform(m_tween, m_position, transformMatrix);

						matrix->a = transformMatrix.a * baseMatrix.a + transformMatrix.c * baseMatrix.b;
						matrix->d = transformMatrix.d * baseMatrix.d + transformMatrix.b * baseMatrix.c;

						matrix->b = baseMatrix.a * transformMatrix.b + baseMatrix.b * transformMatrix.d;
						matrix->c = baseMatrix.c * transformMatrix.a + baseMatrix.d * transformMatrix.c;

						matrix->tx = transformMatrix.a * baseMatrix.tx + transformMatrix.c * baseMatrix.ty + transformMatrix.tx;
						matrix->ty = transformMatrix.b * baseMatrix.tx + transformMatrix.d * baseMatrix.ty + transformMatrix.ty;
					}
				}
				else if (m_matrixTweener) {
					matrix = new DOM::Utils::MATRIX2D();
					m_matrixTweener->GetGeometricTransform(m_tween, m_position, *matrix);
				}

				if (m_colorTweener) {
					color = new DOM::Utils::COLOR_MATRIX();
					m_colorTweener->GetColorMatrix(m_tween, m_position, *color);
				}
				else if (m_colors[i]) {
					color = m_colors[i].get();
				}

				writer->AddFrameElement(
					get<0>(m_elementsData[i]),
					get<1>(m_elementsData[i]),
					get<2>(m_elementsData[i]),
					matrix,
					color
				);
			}
		}
	}
}