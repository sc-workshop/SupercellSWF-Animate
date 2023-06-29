#include "Publisher/TimelineBuilder/FrameBuilder.h"

#include "Publisher/ResourcePublisher.h"

// Textfields
#include "DOM/FrameElement/IClassicText.h"
#include "DOM/FrameElement/ITextStyle.h"

// Filters
#include "DOM/IFilterable.h"
#include "DOM/GraphicFilter/IGlowFilter.h"

namespace sc {
	namespace Adobe {
		void FrameBuilder::Update(FCM::AutoPtr<DOM::IFrame>& frame) {
			if (!frame) {
				throw exception("Failed to get NULL frame");
			}

			// cleaning
			m_duration = 0;
			m_position = 0;
			m_label = u"";
			m_elementsData.clear();
			m_matrices.clear();
			m_colors.clear();
			m_tween = nullptr;
			m_matrixTweener = nullptr;
			m_colorTweener = nullptr;

			DOM::KeyFrameLabelType labelType = DOM::KeyFrameLabelType::KEY_FRAME_LABEL_NONE;
			frame->GetLabelType(labelType);
			if (labelType == DOM::KeyFrameLabelType::KEY_FRAME_LABEL_NAME) {
				FCM::StringRep16 frameNamePtr;
				frame->GetLabel(&frameNamePtr);
				m_label = (const char16_t*)frameNamePtr;
				m_resources.context.falloc->Free(frameNamePtr);
			}

			frame->GetDuration(m_duration);

			// Frame elements processing
			FCM::FCMListPtr frameElements;
			uint32_t frameElementsCount = 0;
			frame->GetFrameElements(frameElements.m_Ptr);
			frameElements->Count(frameElementsCount);

			for (uint32_t i = 0; frameElementsCount > i; i++) {
				FCM::AutoPtr<DOM::FrameElement::IFrameDisplayElement> frameElement = frameElements[i];
				if (!frameElement) {
					throw exception("Failed to get frame element");
				}

				// Symbol info
				uint16_t id = 0xFFFF;
				u16string name = u"";
				uint8_t blendMode = 0; // TODO

				// Base transform
				MATRIX2D* matrix = NULL;
				COLOR_MATRIX* color = NULL;

				// Game "guess who i am"
				FCM::AutoPtr<DOM::FrameElement::IInstance> libraryElement = frameElement;
				FCM::AutoPtr<DOM::IFilterable> filterableElement = frameElement;
				FCM::AutoPtr<DOM::FrameElement::IClassicText> textfieldElement = frameElement;

				FCM::AutoPtr<DOM::FrameElement::IMovieClip> movieClipElement = frameElement;
				FCM::AutoPtr<DOM::FrameElement::ISymbolInstance> symbolItem = frameElement;

				// Symbol
				if (libraryElement) {
					DOM::PILibraryItem libraryItem;
					libraryElement->GetLibraryItem(libraryItem);

					FCM::StringRep16 itemNamePtr;
					libraryItem->GetName(&itemNamePtr);
					u16string itemName = (const char16_t*)itemNamePtr;
					m_resources.context.falloc->Free(itemNamePtr);

					id = m_resources.GetIdentifer(itemName);
					if (id == UINT16_MAX) {
						id = m_resources.AddLibraryItem(itemName, libraryItem);
					}

					m_resources.GetSymbolUsage(itemName)++;

					// Transform
					matrix = new MATRIX2D();
					frameElement->GetMatrix(*matrix);

					if (symbolItem) {
						color = new COLOR_MATRIX();
						symbolItem->GetColorMatrix(*color);
					}

					if (movieClipElement) {
						// Instance name
						FCM::StringRep16 instanceNamePtr;
						movieClipElement->GetName(&instanceNamePtr);
						name = (const char16_t*)instanceNamePtr;
						m_resources.context.falloc->Free(instanceNamePtr);
					}
				}

				// Textfield
				else if (textfieldElement) {
					matrix = new MATRIX2D();
					frameElement->GetMatrix(*matrix);

					TextFieldInfo textfield;

					{
						frameElement->GetObjectSpaceBounds(textfield.bound);

						FCM::StringRep16 text;
						textfieldElement->GetText(&text);
						textfield.text = u16string((const char16_t*)text);
						m_resources.context.falloc->Free(text);

						textfield.renderingMode.structSize = sizeof(textfield.renderingMode);
						textfieldElement->GetAntiAliasModeProp(textfield.renderingMode);

						FCM::AutoPtr<DOM::FrameElement::ITextBehaviour> textfieldElementBehaviour;
						textfieldElement->GetTextBehaviour(textfieldElementBehaviour.m_Ptr);

						// Instance name

						FCM::AutoPtr<DOM::FrameElement::IModifiableTextBehaviour> modifiableTextfieldBehaviour = textfieldElementBehaviour;
						if (modifiableTextfieldBehaviour) {
							FCM::StringRep16 instanceName;
							modifiableTextfieldBehaviour->GetInstanceName(&instanceName);
							name = (const char16_t*)instanceName;
							m_resources.context.falloc->Free(instanceName);

							modifiableTextfieldBehaviour->GetLineMode(textfield.lineMode);
						}

						// Textfields properties

						FCM::FCMListPtr paragraphs;
						uint32_t paragraphsCount = 0;
						textfieldElement->GetParagraphs(paragraphs.m_Ptr);
						paragraphs->Count(paragraphsCount);

						ASSERT(paragraphsCount >= 1);
						if (paragraphsCount > 1) {
							m_resources.context.trace("Warning. Some of TextField has multiple paragraph");
						}

						FCM::AutoPtr<DOM::FrameElement::IParagraph> paragraph = paragraphs[0];
						textfield.style.structSize = sizeof(textfield.style);
						paragraph->GetParagraphStyle(textfield.style);

						FCM::FCMListPtr textRuns;
						uint32_t textRunCount = 0;
						paragraph->GetTextRuns(textRuns.m_Ptr);
						textRuns->Count(textRunCount);

						ASSERT(textRunCount >= 1);
						if (textRunCount > 1) {
							m_resources.context.trace("Warning. Some of TextField has multiple textRun");
						}

						FCM::AutoPtr<DOM::FrameElement::ITextRun> textRun = textRuns[0];
						FCM::AutoPtr<DOM::FrameElement::ITextStyle> textStyle;
						textRun->GetTextStyle(textStyle.m_Ptr);

						textStyle->GetFontColor(textfield.fontColor);
						textStyle->GetFontSize(textfield.fontSize);
						textStyle->IsAutoKernEnabled(textfield.autoKern);

						FCM::StringRep16 fontNamePtr;
						textStyle->GetFontName(&fontNamePtr);
						textfield.fontName = u16string((const char16_t*)fontNamePtr);
						m_resources.context.falloc->Free(fontNamePtr);

						FCM::StringRep8 fontStylePtr;
						textStyle->GetFontStyle(&fontStylePtr);
						textfield.fontStyle = string((const char*)fontStylePtr);
						m_resources.context.falloc->Free(fontStylePtr);
					}

					// Textfield filters
					if (filterableElement) {
						FCM::FCMListPtr filters;
						filterableElement->GetGraphicFilters(filters.m_Ptr);
						uint32_t filterCount = 0;
						filters->Count(filterCount);
						
						for (uint32_t i = 0; filterCount > i; i++) {
							// And again game "guess who i am"
							FCM::AutoPtr<DOM::GraphicFilter::IGlowFilter> glowFilter = filters[i];

							if (glowFilter) {
								textfield.isOutlined = true;
								glowFilter->GetShadowColor(textfield.outlineColor);
							}
						}
					}

					id = m_resources.GetIdentifer(textfield);
					if (id == UINT16_MAX) {
						id = m_resources.AddTextField(textfield);
					}
				}
				else {
					m_resources.context.trace("Unknown resource in scene");
				}

				if (id == 0xFFFF) {
					throw exception("Failed to get frame element id");
				}

				m_elementsData.push_back({
					id,
					blendMode,
					name
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
				FCM::PIFCMDictionary tweenerDict;
				m_tween->GetTweenedProperties(tweenerDict);

				FCM::FCMGUID matrixGuid;
				FCM::FCMGUID colorGuid;

				bool hasMatrixTweener = Utils::ReadGUID(tweenerDict, kDOMGeometricProperty, matrixGuid);
				bool hasColorTweener = Utils::ReadGUID(tweenerDict, kDOMColorProperty, colorGuid);

				FCM::AutoPtr<ITweenerService> TweenerService = m_resources.context.getService<ITweenerService>(TWEENER_SERVICE);

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
			if (!m_label.empty()) {
				writer->SetLabel(m_label);
			}

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