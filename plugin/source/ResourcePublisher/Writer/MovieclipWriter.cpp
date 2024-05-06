#include "ResourcePublisher/Writer/Writer.h"
#include "ResourcePublisher/Writer/MovieclipWriter.h"
#include "Module/PluginContext.h"

namespace sc {
	namespace Adobe {
		SCMovieclipWriter::SCMovieclipWriter(SCWriter& writer, SymbolContext& symbol)
			: m_writer(writer), m_symbol(symbol) {};

		void SCMovieclipWriter::InitializeTimeline(double fps, uint32_t frameCount) {
			m_object.frame_rate = (uint8_t)fps;
			m_object.frames.resize(frameCount);

			if (m_symbol.slice_scaling.IsEnabled())
			{
				DOM::Utils::RECT guides = m_symbol.slice_scaling.Guides();

				m_object.use_nine_slice = true;
				m_object.scaling_grid.x = guides.topLeft.y;
				m_object.scaling_grid.y = guides.topLeft.x;
				m_object.scaling_grid.width = guides.bottomRight.y - guides.topLeft.y;
				m_object.scaling_grid.height = guides.bottomRight.x - guides.topLeft.x;
			}
		}

		uint16_t SCMovieclipWriter::GetInstanceIndex(
			uint16_t elementsCount,
			uint16_t id,
			FCM::BlendMode _blending,
			std::string name
		) {
			DisplayObjectInstance::BlendMode blending = (DisplayObjectInstance::BlendMode)(_blending);

			uint16_t frameInstancesOffset = 0;
			uint32_t frameElementsLastIndex = m_object.frame_elements.size() - 1;
			for (uint32_t i = 0; elementsCount > i; i++) {
				uint16_t& instanceIndex = m_object.frame_elements[frameElementsLastIndex - i].instance_index;
				DisplayObjectInstance& instance = m_object.instances[instanceIndex];

				if (
					instance.id == id &&
					instance.name.string() == name &&
					instance.blend_mode == blending) {
					frameInstancesOffset++;
				}
			}

			uint16_t instancesCount = m_object.instances.size();
			for (uint16_t i = 0; instancesCount > i; i++) {
				DisplayObjectInstance& instance = m_object.instances[i];

				if (instance.id == id &&
					instance.name.string() == name &&
					instance.blend_mode == (DisplayObjectInstance::BlendMode)blending) {
					if (frameInstancesOffset != 0) {
						frameInstancesOffset--;
						continue;
					}

					return i;
				}
			}

			DisplayObjectInstance& instance = m_object.instances.emplace_back();
			instance.id = id;
			instance.blend_mode = blending;
			instance.name = SWFString(name);

			return instancesCount;
		}

		void SCMovieclipWriter::SetLabel(const std::u16string& _label) {
			std::string label = Localization::ToUtf8(_label);
			m_object.frames[m_position].label = SWFString(label);
		}

		void SCMovieclipWriter::AddFrameElement(
			uint16_t id,
			FCM::BlendMode blending,
			const std::u16string& name,

			std::optional<DOM::Utils::MATRIX2D> matrix,
			std::optional<DOM::Utils::COLOR_MATRIX> color
		) {
			MovieClipFrame& frame = m_object.frames[m_position];

			// Index of bind element
			uint16_t instanceIndex = GetInstanceIndex(
				frame.elements_count,
				id, blending, Localization::ToUtf8(name)
			);

			// New frame element
			MovieClipFrameElement& element = m_object.frame_elements.emplace_back();
			element.instance_index = instanceIndex;

			frame.elements_count += 1;

			// matrices.size = colors.size = object.frameElements.size
			std::optional<Matrix2D> transformMatrix = std::nullopt;
			std::optional<ColorTransform> transformColor = std::nullopt;

			if (matrix) {
				transformMatrix = Matrix2D();

				transformMatrix->a = matrix->a;
				transformMatrix->b = matrix->b;
				transformMatrix->c = matrix->c;
				transformMatrix->d = matrix->d;
				transformMatrix->tx = matrix->tx;
				transformMatrix->ty = matrix->ty;
			}

			if (color) {
				transformColor = ColorTransform();

				transformColor->alpha = (uint8_t)std::clamp(
					(int)((color->matrix[3][3] * 255) + color->matrix[3][4]),
					0, 255
				);

				transformColor->multiply.r = (uint8_t)std::clamp(int(color->matrix[0][0] * 255), 0, 255);
				transformColor->multiply.g = (uint8_t)std::clamp(int(color->matrix[1][1] * 255), 0, 255);
				transformColor->multiply.b = (uint8_t)std::clamp(int(color->matrix[2][2] * 255), 0, 255);

				transformColor->add.r = (uint8_t)std::clamp(int(color->matrix[0][4]), 0, 255);
				transformColor->add.g = (uint8_t)std::clamp(int(color->matrix[1][4]), 0, 255);
				transformColor->add.b = (uint8_t)std::clamp(int(color->matrix[2][4]), 0, 255);
			}

			m_matrices.push_back(transformMatrix);
			m_colors.push_back(transformColor);
		}

		bool SCMovieclipWriter::Finalize(uint16_t id) {
			m_object.id = id;

			if (m_symbol.linkage_name.empty())
			{
				if (m_object.instances.empty())
				{
					return false;
				}
			}
			else
			{
				ExportName& export_name = m_writer.swf.exports.emplace_back();
				export_name.name = SWFString(m_symbol.linkage_name);
				export_name.id = id;
			}

			FinalizeTransforms();

			m_writer.swf.movieclips.push_back(m_object);

			return true;
		}

		void SCMovieclipWriter::FinalizeTransforms() {
			PluginContext& context = PluginContext::Instance();
			// TODO: Rework this..?

			// Transforms packing
			uint8_t matrixBanksCount = m_writer.swf.matrixBanks.size();

			// First, iterate through all banks
			for (uint8_t i = 0; matrixBanksCount > i; i++) {
				if (FinalizeElementsTransform(i)) {
					return;
				}
			}

			// If all banks is full then create new
			m_writer.swf.matrixBanks.emplace_back();
			if (!FinalizeElementsTransform(matrixBanksCount)) {
				// and even if the new bank cannot cope with all transformations, then we just give up and delete everything
				m_object.frames.clear();
				m_object.frame_elements.clear();
				m_object.instances.clear();

				context.Trace("Failed to pack transfroms into MovieClip \"%s\", so it will be empty", m_symbol.name);
			}
		}

		bool SCMovieclipWriter::FinalizeElementsTransform(uint8_t& bankIndex) {
			uint32_t frameElementsCount = m_object.frame_elements.size();

			MatrixBank& bankOriginal = m_writer.swf.matrixBanks[bankIndex];
			MatrixBank bankCurrent = MatrixBank(bankOriginal);

			for (uint32_t i = 0; frameElementsCount > i; i++) {
				if (bankCurrent.matrices.size() >= 0xFFFE || bankCurrent.color_transforms.size() >= 0xFFFE) return false;

				uint16_t matrixIndex = 0xFFFF;
				uint16_t colorIndex = 0xFFFF;

				if (m_matrices[i]) {
					if (!bankCurrent.get_matrix_index(*m_matrices[i], matrixIndex)) {
						matrixIndex = bankCurrent.matrices.size();
						bankCurrent.matrices.push_back(*m_matrices[i]);
					}
				}

				if (m_colors[i]) {
					if (!bankCurrent.get_colorTransform_index(*m_colors[i], colorIndex)) {
						colorIndex = bankCurrent.color_transforms.size();
						bankCurrent.color_transforms.push_back(*m_colors[i]);
					}
				}

				m_object.frame_elements[i].matrix_index = matrixIndex;
				m_object.frame_elements[i].colorTransform_index = colorIndex;
			}

			m_object.bank_index = bankIndex;
			m_writer.swf.matrixBanks[bankIndex] = bankCurrent;
			return true;
		}
	}
}