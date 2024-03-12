#pragma once

#include "ResourcePublisher/Writer/Shared/SharedGraphicWriter.h"
#include "AnimateSDK/app/ApplicationFCMPublicIDs.h"

namespace sc {
	namespace Adobe {
		class SCWriter;
		struct Sprite;

		class SCShapeWriter : public SharedShapeWriter {
			SCWriter& m_writer;
			SymbolContext& m_symbol;

			std::vector<Sprite> m_sprites;

		public:
			SCShapeWriter(SCWriter& writer, SymbolContext& symbol) : m_writer(writer), m_symbol(symbol) {};
			virtual ~SCShapeWriter() = default;

			void AddGraphic(const cv::Mat& image, const DOM::Utils::MATRIX2D& matrix);

			void AddSolidColorRegion(const FilledShapePath& contour, const std::vector<FilledShapePath>& holes, const cv::Scalar& color);
			void AddRasterizedSolidColorRegion(const FilledShapePath& contour, const std::vector<FilledShapePath>& holes, const cv::Scalar& color);

			void AddFilledShape(const FilledShape& shape);
			void AddFilledShapeRegion(const FilledShapeRegion& region);

			void Finalize(uint16_t id);
		};
	}
}