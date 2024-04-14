#pragma once

#include "ResourcePublisher/Writer/Shared/SharedGraphicWriter.h"
#include "GraphicItem/GraphicGroup.h"
#include "GraphicItem/SpriteItem.h"
#include "AnimateSDK/app/ApplicationFCMPublicIDs.h"

namespace sc {
	namespace Adobe {
		class SCWriter;
		struct Sprite;

		class SCShapeWriter : public SharedShapeWriter {
			SCWriter& m_writer;
			SymbolContext& m_symbol;

			GraphicGroup m_group;

		public:
			SCShapeWriter(SCWriter& writer, SymbolContext& symbol) : m_writer(writer), m_symbol(symbol), m_group(symbol) {};
			virtual ~SCShapeWriter() = default;

			void AddGraphic(const SpriteElement& item, const DOM::Utils::MATRIX2D& matrix);

			void AddSolidColorRegion(const FilledElementPath& contour, const std::vector<FilledElementPath>& holes, const cv::Scalar& color);
			void AddRasterizedSolidColorRegion(const FilledElementPath& contour, const std::vector<FilledElementPath>& holes, const cv::Scalar& color);

			void AddFilledShape(const FilledElement& shape);
			void AddFilledShapeRegion(const FilledElementRegion& region);

			void Finalize(uint16_t id);
		};
	}
}