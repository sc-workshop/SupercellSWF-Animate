#include "Publisher/SWF/Writer.h"
#include "Publisher/SWF/MovieclipWriter.h"

#include "Macros.h"

using namespace FCM;

namespace sc {
	namespace Adobe {
		Result MovieclipWriter::Init(Writer* writer, PIFCMCallback callback) {
			Result res = FCM_SUCCESS;

			m_callback = callback;

			if (writer) {
				m_writer = writer;
			}
			else {
				return FCM_EXPORT_FAILED;
			}

			console.Init("Timeline", m_callback);

			return FCM_SUCCESS;
		}

		uint16_t MovieclipWriter::GetInstanceIndex(
			U_Int32 elementsOffset,
			U_Int16 elementsCount,
			U_Int16 id,
			U_Int8 blending,
			std::string name
		) {

			uint16_t frameInstancesOffset = 0;
			for (uint32_t i = 0; elementsCount > i; i++) {
				uint16_t& instanceIndex = m_object->frameElements[elementsOffset + i]->instanceIndex;
				pDisplayObjectInstance& instance = m_object->instances[instanceIndex];

				if (
					instance->id == id &&
					instance->name == name &&
					instance->blend == blending) {
					frameInstancesOffset++;
				}
			}

			uint16_t instancesCount = (uint16_t)m_object->instances.size();
			for (uint16_t i = 0; instancesCount > i; i++) {
				pDisplayObjectInstance& instance = m_object->instances[i];

				if (instance->id == id &&
					instance->name == name &&
					instance->blend == blending) {
					if (frameInstancesOffset != 0) {
						frameInstancesOffset--;
						continue;
					}

					return i;
				}
			}

			DisplayObjectInstance* instance = new DisplayObjectInstance();
			instance->id = id;
			instance->blend = blending;
			instance->name = name;

			m_object->instances.push_back(
				pDisplayObjectInstance(instance)
			);

			return instancesCount;
		}

		Result MovieclipWriter::InitTimeline(U_Int32 frameCount) {
			for (U_Int32 i = 0; frameCount > i; i++) {
				m_object->frames.push_back(
					pMovieClipFrame(new MovieClipFrame())
				);
			}
			return FCM_SUCCESS;
		}

		Result MovieclipWriter::SetLabel(U_Int32 frameIndex, std::string label) {
			if (m_object->frames.size() > frameIndex) return FCM_EXPORT_FAILED;

			m_object->frames[frameIndex]->label(label);
			return FCM_SUCCESS;
		}

		Result MovieclipWriter::AddFrameElement(
			U_Int32 frameIndex,

			U_Int16 id,
			U_Int8 blending,
			std::string name,

			DOM::Utils::MATRIX2D& matrix,
			DOM::Utils::COLOR_MATRIX& color
		) {
			if (m_object->frames.size() < frameIndex) return FCM_EXPORT_FAILED;

			// Frame elements offset in flat vector
			uint32_t elementsOffset = 0;
			for (uint16_t i = 0; frameIndex > i; i++) {
				elementsOffset += m_object->frames[i]->elementsCount();
			}

			pMovieClipFrame& frame = m_object->frames[frameIndex];
			uint16_t elementsCount = frame->elementsCount();

			// Index of bind element
			U_Int16 instanceIndex = GetInstanceIndex(
				elementsOffset,
				elementsCount,
				id, blending, name);

			// New frame element
			pMovieClipFrameElement element = pMovieClipFrameElement(new MovieClipFrameElement());
			element->instanceIndex = instanceIndex;

			m_object->frameElements.insert(
				m_object->frameElements.begin() + (elementsOffset + elementsCount),
				element
			);
			frame->elementsCount(elementsCount + 1);

			// matrices.size = colors.size = object.frameElements.size

			// May be unsafe but this cast must be faster
			Matrix2D* movieMatrix = new Matrix2D();
			memcpy(movieMatrix, &matrix, sizeof(*movieMatrix));

			m_matrices.insert(
				m_matrices.begin() + (elementsOffset + elementsCount),
				pMatrix2D(movieMatrix)
			);

			// Color
			pColorTransform movieColor(new ColorTransform());

			movieColor->alpha = (uint8_t)clamp(
				(int)((color.matrix[3][3] * 255) + color.matrix[3][4]),
				0, 255
			);

			movieColor->redMul = (uint8_t)clamp(int(color.matrix[0][0] * 255), 0, 255);
			movieColor->greenMul = (uint8_t)clamp(int(color.matrix[1][1] * 255), 0, 255);
			movieColor->blueMul = (uint8_t)clamp(int(color.matrix[2][2] * 255), 0, 255);

			movieColor->redAdd = (uint8_t)clamp(int(color.matrix[0][4]), 0, 255);
			movieColor->greenAdd = (uint8_t)clamp(int(color.matrix[1][4]), 0, 255);
			movieColor->blueAdd = (uint8_t)clamp(int(color.matrix[2][4]), 0, 255);

			m_colors.insert(
				m_colors.begin() + (elementsOffset + elementsCount),
				movieColor
			);

			return FCM_SUCCESS;
		}

		void MovieclipWriter::Finalize(U_Int16 id, U_Int8 fps, std::string name) {
			//TODO: export name filtering
			m_object->id(id);
			m_object->frameRate(fps);
			
			if (!name.empty()) {
				pExportName exportName = pExportName(new ExportName());
				exportName->id(id);
				exportName->name(name);
				m_writer->swf.exports.push_back(exportName);
			}

			FinalizeTransforms();

			m_writer->swf.movieClips.push_back(m_object);
		}

		void MovieclipWriter::FinalizeTransforms() {
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

		bool MovieclipWriter::FinalizeElementsTransform(uint8_t& bankIndex) {
			uint32_t frameElementsCount = (uint32_t)m_object->frameElements.size();

			pMatrixBank& bankOrig = m_writer->swf.matrixBanks[bankIndex];
			pMatrixBank& bankCopy = pMatrixBank(new MatrixBank(*bankOrig.get()));

			for (uint32_t i = 0; frameElementsCount > i; i++) {
				if (bankCopy->matrices.size() >= 0xFFFE || bankCopy->colorTransforms.size() >= 0xFFFE) return false;

				uint16_t matrixIndex = 0xFFFF;
				uint16_t colorIndex = 0xFFFF;

				if (!bankCopy->getMatrixIndex(m_matrices[i].get(), matrixIndex)) {
					matrixIndex = (uint16_t)bankCopy->matrices.size();
					bankCopy->matrices.push_back(m_matrices[i]);
				}

				if (!bankCopy->getColorTransformIndex(m_colors[i].get(), colorIndex)) {
					colorIndex = (uint16_t)bankCopy->colorTransforms.size();
					bankCopy->colorTransforms.push_back(m_colors[i]);
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