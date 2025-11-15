#include "MovieclipWriter.h"

#include "Writer/Writer.h"
#include "Module/Module.h"
#include "GraphicItem/SlicedItem.h"

#include "core/hashing/ncrypto/xxhash.h"
#include "core/hashing/hash.h"

using namespace Animate::Publisher;

namespace sc {
	namespace Adobe {
		SCMovieclipWriter::SCMovieclipWriter(SCWriter& writer, SymbolContext& symbol) :
			Animate::Publisher::SharedMovieclipWriter(symbol),
			m_writer(writer)
		{
		};

		SCMovieclipWriter::~SCMovieclipWriter()
		{
			SCPlugin& context = SCPlugin::Instance();

			if (m_status)
			{
				context.Window()->DestroyStatusBar(m_status);
			}
		}

		void SCMovieclipWriter::InitializeTimeline(uint32_t fps, uint32_t frameCount) {
			SCPlugin& context = SCPlugin::Instance();

			m_object.frame_rate = (uint8_t)fps;
			m_object.frames.resize(frameCount);

			if (!m_status && frameCount > 100)
			{
				m_status = context.Window()->CreateStatusBarComponent(m_symbol.name);
				m_status->SetRange(frameCount);
			}

			if (m_symbol.slicing.IsEnabled())
			{
				auto& guides = m_symbol.slicing.Guides();

				m_object.scaling_grid = wk::RectF();
				m_object.scaling_grid->left = guides.topLeft.y;
				m_object.scaling_grid->top = guides.topLeft.x;
				m_object.scaling_grid->right = guides.bottomRight.y;
				m_object.scaling_grid->bottom = guides.bottomRight.x;
			}
		}

		void SCMovieclipWriter::Next()
		{
			SharedMovieclipWriter::Next();

			if (m_status)
			{
				m_status->SetProgress(m_position);
			}
		}

		uint16_t SCMovieclipWriter::GetInstanceIndex(
			uint16_t elementsCount,
			uint16_t id,
			FCM::BlendMode _blending,
			std::string name
		) {
			flash::DisplayObjectInstance::BlendMode blending = (flash::DisplayObjectInstance::BlendMode)(_blending);

			uint16_t frameInstancesOffset = 0;
			uint32_t frameElementsLastIndex = m_object.frame_elements.size() - 1;
			for (uint32_t i = 0; elementsCount > i; i++) {
				uint16_t& instanceIndex = m_object.frame_elements[frameElementsLastIndex - i].instance_index;
				flash::DisplayObjectInstance& instance = m_object.childrens[instanceIndex];

				if (
					instance.id == id &&
					instance.name.string() == name &&
					instance.blend_mode == blending) {
					frameInstancesOffset++;
				}
			}

			uint16_t instancesCount = m_object.childrens.size();
			for (uint16_t i = 0; instancesCount > i; i++) {
				flash::DisplayObjectInstance& instance = m_object.childrens[i];

				if (instance.id == id &&
					instance.name.string() == name &&
					instance.blend_mode == (flash::DisplayObjectInstance::BlendMode)blending) {
					if (frameInstancesOffset != 0) {
						frameInstancesOffset--;
						continue;
					}

					return i;
				}
			}

			flash::DisplayObjectInstance& instance = m_object.childrens.emplace_back();
			instance.id = id;
			instance.blend_mode = blending;
			instance.name = flash::SWFString(name);

			return instancesCount;
		}

		void SCMovieclipWriter::SetLabel(const std::u16string& _label) {
			std::string label = FCM::Locale::ToUtf8(_label);
			m_object.frames[m_position].label = flash::SWFString(label);
		}

		void SCMovieclipWriter::AddFrameElement(
			uint16_t id,
			FCM::BlendMode blending,
			const std::u16string& name,

			std::optional<Animate::DOM::Utils::MATRIX2D> matrix,
			std::optional<Animate::DOM::Utils::COLOR_MATRIX> color
		) {
			flash::MovieClipFrame& frame = m_object.frames[m_position];

			// Index of bind element
			uint16_t instanceIndex = GetInstanceIndex(
				frame.elements_count,
				id, blending, FCM::Locale::ToUtf8(name)
			);

			// New frame element
			flash::MovieClipFrameElement& element = m_object.frame_elements.emplace_back();
			element.instance_index = instanceIndex;

			frame.elements_count += 1;

			if (matrix) {
				flash::Matrix2D transformMatrix;

				transformMatrix.a = matrix->a;
				transformMatrix.b = matrix->b;
				transformMatrix.c = matrix->c;
				transformMatrix.d = matrix->d;
				transformMatrix.tx = matrix->tx;
				transformMatrix.ty = matrix->ty;

				if (!m_bank.get_matrix_index(transformMatrix, element.matrix_index))
				{
					element.matrix_index = m_bank.matrices.size();
					m_bank.matrices.push_back(transformMatrix);
				}
			}

			if (color) {
				flash::ColorTransform transformColor;

				transformColor.alpha = (uint8_t)std::clamp(
					(int)((color->matrix[3][3] * 255) + color->matrix[3][4]),
					0, 255
				);

				transformColor.multiply.r = (uint8_t)std::clamp(int(color->matrix[0][0] * 255), 0, 255);
				transformColor.multiply.g = (uint8_t)std::clamp(int(color->matrix[1][1] * 255), 0, 255);
				transformColor.multiply.b = (uint8_t)std::clamp(int(color->matrix[2][2] * 255), 0, 255);

				transformColor.add.r = (uint8_t)std::clamp(int(color->matrix[0][4]), 0, 255);
				transformColor.add.g = (uint8_t)std::clamp(int(color->matrix[1][4]), 0, 255);
				transformColor.add.b = (uint8_t)std::clamp(int(color->matrix[2][4]), 0, 255);

				if (!m_bank.get_colorTransform_index(transformColor, element.colorTransform_index))
				{
					element.colorTransform_index = m_bank.color_transforms.size();
					m_bank.color_transforms.push_back(transformColor);
				}
			}
		}

		std::size_t SCMovieclipWriter::GenerateHash() const
		{
			wk::hash::XxHash code;

			code.update(m_object.frame_rate);
			code.update(
				(const uint8_t*)m_object.frame_elements.data(),
				sizeof(flash::MovieClipFrameElement) * m_object.frame_elements.size()
			);

			for (auto& children : m_object.childrens)
			{
				code.update(children.id);
				code.update(children.blend_mode);
				code.update(children.name);
			}

			for (const auto& frame : m_object.frames)
			{
				code.update(frame.label);
				code.update(frame.elements_count);
			}

			if (m_object.scaling_grid.has_value())
			{
				code.update(m_object.scaling_grid.value());
			}

			code.update(m_bank);

			return code.digest();
		}

		bool SCMovieclipWriter::Finalize(uint16_t id, bool required, bool new_symbol) {
			m_object.id = id;

			if (m_symbol.linkage_name.empty())
			{
				if (m_object.childrens.empty() && !required)
				{
					return false;
				}
			}
			else
			{
				m_writer.IncrementSymbolsProcessed();
				m_writer.swf.CreateExportName(
					flash::SWFString(m_symbol.linkage_name),
					id
				);

				if (!new_symbol) return true;
			}

			FinalizeTransforms();
			m_writer.swf.movieclips.push_back(m_object);

			return true;
		}

		void SCMovieclipWriter::FinalizeTransforms() {
			if (m_writer.swf.matrixBanks.empty())
			{
			CREATE_NEW_BANK:
				m_object.bank_index = m_writer.swf.matrixBanks.size();
				m_writer.swf.matrixBanks.push_back(m_bank);
				return;
			}
			
			uint32_t bank_index = 0;
			flash::SWFVector<uint16_t, uint32_t> matrix_indices;
			flash::SWFVector<uint16_t, uint32_t> color_indices;
			matrix_indices.reserve(m_bank.matrices.size());
			color_indices.reserve(m_bank.color_transforms.size());

			uint16_t new_matrices_count = 0;
			uint16_t new_colors_count = 0;

			for (uint32_t i = 0; m_writer.swf.matrixBanks.size() > i; i++)
			{
				flash::MatrixBank& bank = m_writer.swf.matrixBanks[i];

				matrix_indices.clear();
				color_indices.clear();
				new_matrices_count = 0;
				new_colors_count = 0;

				for (flash::Matrix2D& matrix : m_bank.matrices)
				{
					uint16_t new_matrix_index = 0xFFFF;
					bool isMatricesFull = bank.matrices.size() + new_matrices_count >= 0xFFFF;
					if (isMatricesFull)
					{
						goto NEXT_BANK;
					};

					if (!bank.get_matrix_index(matrix, new_matrix_index))
					{
						new_matrices_count++;
					}

					matrix_indices.push_back(new_matrix_index);
				}

				for (flash::ColorTransform& color : m_bank.color_transforms)
				{
					uint16_t new_color_index = 0xFFFF;
					bool isColorsFull = bank.color_transforms.size() + new_colors_count >= 0xFFFF;
					if (isColorsFull)
					{
						goto NEXT_BANK;
					};

					if (!bank.get_colorTransform_index(color, new_color_index))
					{
						new_colors_count++;
					}

					color_indices.push_back(new_color_index);
				}

				bank_index = i;
				bank.matrices.reserve(bank.matrices.size() + new_matrices_count);
				bank.color_transforms.reserve(bank.color_transforms.size() + new_colors_count);
				break;

			NEXT_BANK:
				if (i == m_writer.swf.matrixBanks.size() - 1)
				{
					goto CREATE_NEW_BANK;
				}
				else
				{
					continue;
				}
			}

			m_object.bank_index = bank_index;
			flash::MatrixBank& bank = m_writer.swf.matrixBanks[m_object.bank_index];
			for (flash::MovieClipFrameElement& element : m_object.frame_elements)
			{
				if (element.matrix_index != 0xFFFF)
				{
					uint16_t& matrix_index = matrix_indices[element.matrix_index];

					if (matrix_index == 0xFFFF)
					{
						matrix_index = bank.matrices.size();
						bank.matrices.push_back(m_bank.matrices[element.matrix_index]);
					}

					element.matrix_index = matrix_index;
				}

				if (element.colorTransform_index != 0xFFFF)
				{
					uint16_t& color_index = color_indices[element.colorTransform_index];

					if (color_index == 0xFFFF)
					{
						color_index = bank.color_transforms.size();
						bank.color_transforms.push_back(m_bank.color_transforms[element.colorTransform_index]);
					}

					element.colorTransform_index = color_index;
				}
			}
		}
	}
}

namespace wk::hash
{
	template<>
	struct Hash_t<sc::flash::MatrixBank>
	{
		template<typename T>
		static void update(wk::hash::HashStream<T>& stream, const sc::flash::MatrixBank& bank)
		{
			for (auto& matrix : bank.matrices)
			{
				stream.update((const uint8_t*)&matrix.elements, sizeof(matrix.elements));
			}

			for (auto& color : bank.color_transforms)
			{
				stream.update((const uint8_t*)&color.add.channels, sizeof(color.add.channels));
				stream.update((const uint8_t*)&color.multiply.channels, sizeof(color.multiply.channels));
				stream.update(color.alpha);
			}
		}
	};
}