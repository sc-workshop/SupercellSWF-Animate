#pragma once

#include "ResourcePublisher/Writer/Shared/SharedGraphicWriter.h"
#include "GraphicItem/GraphicGroup.h"
#include "GraphicItem/SpriteItem.h"
#include "GraphicItem/FilledItem.h"
#include "AnimateSDK/app/ApplicationFCMPublicIDs.h"

namespace sc {
	namespace Adobe {
		class SCWriter;
		struct Sprite;

		class SCShapeWriter : public SharedShapeWriter {
		public:
			SCShapeWriter(SCWriter& writer, SymbolContext& symbol) : m_writer(writer), m_symbol(symbol), m_group(symbol) {};
			virtual ~SCShapeWriter() = default;

		public:

			virtual void AddGraphic(const SpriteElement& item, const DOM::Utils::MATRIX2D& matrix);

			virtual void AddFilledElement(const FilledElement& shape);

			virtual void AddSlicedElements(const std::vector<FilledElement>& elements, const DOM::Utils::RECT& guides);

			virtual void Finalize(uint16_t id);

		public:
			void AddTriangulatedRegion(
				const FilledElementPath& contour,
				const std::vector<FilledElementPath>& holes,
				const DOM::Utils::COLOR& color
			);

			void AddRasterizedRegion(
				const FilledElementRegion& region
			);

		public:

			void AddSolidColorRegion(const FilledElementPath& contour, const std::vector<FilledElementPath>& holes, const cv::Scalar& color);
			void AddRasterizedSolidColorRegion(const FilledElementPath& contour, const std::vector<FilledElementPath>& holes, const cv::Scalar& color);

			void AddFilledShapeRegion(const FilledElementRegion& region);

		private:
			SCWriter& m_writer;
			SymbolContext& m_symbol;

			GraphicGroup m_group;
		};
	}
}