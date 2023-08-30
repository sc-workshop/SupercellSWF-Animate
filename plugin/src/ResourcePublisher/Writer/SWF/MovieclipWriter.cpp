#include "ResourcePublisher/Writer/SWF/Writer.h"
#include "ResourcePublisher/Writer/SWF/MovieclipWriter.h"
#include "Module/Context.h"
#include "Utils.h"

namespace sc {
	namespace Adobe {
		void SCMovieclipWriter::Init(Context&, SymbolBehaviorInfo& info, uint32_t frameCount) {
			for (uint32_t i = 0; frameCount > i; i++) {
				m_object->frames.push_back(
					pMovieClipFrame(new MovieClipFrame())
				);
			}

			if (info.hasSlice9)
			{
				DOM::Utils::RECT& rect = info.slice9;
				ScalingGrid* grid = new ScalingGrid();

				grid->x = rect.topLeft.y;
				grid->y = rect.topLeft.x;
				grid->width = rect.bottomRight.y - rect.topLeft.y;
				grid->height = rect.bottomRight.x - rect.topLeft.x;

				m_object->scalingGrid(
					pScalingGrid(grid)
				);
			}
		}

		uint16_t SCMovieclipWriter::GetInstanceIndex(
			uint16_t elementsCount,
			uint16_t id,
			FCM::BlendMode blending,
			std::string name
		) {
			uint16_t frameInstancesOffset = 0;
			uint32_t frameElementsLastIndex = (uint32_t)m_object->frameElements.size() - 1;
			for (uint32_t i = 0; elementsCount > i; i++) {
				uint16_t& instanceIndex = m_object->frameElements[frameElementsLastIndex - i]->instanceIndex;
				pDisplayObjectInstance& instance = m_object->instances[instanceIndex];

				if (
					instance->id == id &&
					instance->name == name &&
					instance->blend == (DisplayObjectInstance::BlendMode)blending) {
					frameInstancesOffset++;
				}
			}

			uint16_t instancesCount = (uint16_t)m_object->instances.size();
			for (uint16_t i = 0; instancesCount > i; i++) {
				pDisplayObjectInstance& instance = m_object->instances[i];

				if (instance->id == id &&
					instance->name == name &&
					instance->blend == (DisplayObjectInstance::BlendMode)blending) {
					if (frameInstancesOffset != 0) {
						frameInstancesOffset--;
						continue;
					}

					return i;
				}
			}

			DisplayObjectInstance* instance = new DisplayObjectInstance();
			instance->id = id;
			instance->blend = (DisplayObjectInstance::BlendMode)blending;
			instance->name = name;

			m_object->instances.push_back(
				pDisplayObjectInstance(instance)
			);

			return instancesCount;
		}

		void SCMovieclipWriter::SetLabel(std::u16string& label) {
			m_object->frames[m_position]->label(
				Utils::ToUtf8(label)
			);
		}

		void SCMovieclipWriter::AddFrameElement(
			uint16_t id,
			FCM::BlendMode blending,
			std::u16string name,

			DOM::Utils::MATRIX2D* matrix,
			DOM::Utils::COLOR_MATRIX* color
		) {
			pMovieClipFrame& frame = m_object->frames[m_position];
			uint16_t elementsCount = frame->elementsCount();

			// Index of bind element
			uint16_t instanceIndex = GetInstanceIndex(
				elementsCount,
				id, blending, Utils::ToUtf8(name)
			);

			// New frame element
			pMovieClipFrameElement element = pMovieClipFrameElement(new MovieClipFrameElement());
			element->instanceIndex = instanceIndex;

			m_object->frameElements.push_back(element);
			frame->elementsCount(elementsCount + 1);

			// matrices.size = colors.size = object.frameElements.size

			// May be unsafe but this cast must be faster
			pMatrix2D transformMatrix;
			pColorTransform transformColor;

			if (matrix != nullptr) {
				transformMatrix =
					pMatrix2D( // I know it's a bullshit
						new Matrix2D(*((Matrix2D*)matrix))
					);
			}

			if (color != nullptr) {
				transformColor = pColorTransform(new ColorTransform());

				transformColor->alpha = (uint8_t)clamp(
					(int)((color->matrix[3][3] * 255) + color->matrix[3][4]),
					0, 255
				);

				transformColor->redMul = (uint8_t)clamp(int(color->matrix[0][0] * 255), 0, 255);
				transformColor->greenMul = (uint8_t)clamp(int(color->matrix[1][1] * 255), 0, 255);
				transformColor->blueMul = (uint8_t)clamp(int(color->matrix[2][2] * 255), 0, 255);

				transformColor->redAdd = (uint8_t)clamp(int(color->matrix[0][4]), 0, 255);
				transformColor->greenAdd = (uint8_t)clamp(int(color->matrix[1][4]), 0, 255);
				transformColor->blueAdd = (uint8_t)clamp(int(color->matrix[2][4]), 0, 255);
			}

			m_matrices.push_back(transformMatrix);
			m_colors.push_back(transformColor);
		}

		void SCMovieclipWriter::Finalize(uint16_t id, uint8_t fps) {
			m_object->id(id);
			m_object->frameRate(fps);

			FinalizeTransforms();

			m_writer->swf.movieClips.push_back(m_object);
		}

		void SCMovieclipWriter::FinalizeTransforms() {
			// TODO: Rework this..?

			// Transforms packing
			uint8_t matrixBanksCount = (uint8_t)m_writer->swf.matrixBanks.size();
			if (matrixBanksCount == 0) {
				m_writer->swf.matrixBanks.push_back(pMatrixBank(new MatrixBank()));
				matrixBanksCount++;
			}

			// First, iterate through all banks
			for (uint8_t i = 0; matrixBanksCount > i; i++) {
				if (FinalizeElementsTransform(i)) {
					return;
				}
			}

			// If all banks is full then create new
			m_writer->swf.matrixBanks.push_back(pMatrixBank(new MatrixBank()));
			if (!FinalizeElementsTransform(matrixBanksCount)) {
				// and even if the new bank cannot cope with all transformations, then we just give up and delete everything
				m_object->frames.clear();
				m_object->frameElements.clear();
				m_object->instances.clear();
			}
		}

		bool SCMovieclipWriter::FinalizeElementsTransform(uint8_t& bankIndex) {
			uint32_t frameElementsCount = (uint32_t)m_object->frameElements.size();

			pMatrixBank& bankOrig = m_writer->swf.matrixBanks[bankIndex];
			pMatrixBank& bankCopy = pMatrixBank(new MatrixBank(*bankOrig.get()));

			for (uint32_t i = 0; frameElementsCount > i; i++) {
				if (bankCopy->matrices.size() >= 0xFFFE || bankCopy->colorTransforms.size() >= 0xFFFE) return false;

				uint16_t matrixIndex = 0xFFFF;
				uint16_t colorIndex = 0xFFFF;

				if (m_matrices[i]) {
					if (!bankCopy->getMatrixIndex(m_matrices[i].get(), matrixIndex)) {
						matrixIndex = (uint16_t)bankCopy->matrices.size();
						bankCopy->matrices.push_back(m_matrices[i]);
					}
				}

				if (m_colors[i]) {
					if (!bankCopy->getColorTransformIndex(m_colors[i].get(), colorIndex)) {
						colorIndex = (uint16_t)bankCopy->colorTransforms.size();
						bankCopy->colorTransforms.push_back(m_colors[i]);
					}
				}

				m_object->frameElements[i]->matrixIndex = matrixIndex;
				m_object->frameElements[i]->colorTransformIndex = colorIndex;
			}

			m_object->matrixBankIndex(bankIndex);
			m_writer->swf.matrixBanks[bankIndex] = bankCopy;
			return true;
		}
	}
}