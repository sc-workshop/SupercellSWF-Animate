#pragma once

#include "ResourcePublisher/Writer/Shared/SharedGraphicWriter.h"
#include "GraphicItem/GraphicGroup.h"
#include "GraphicItem/SpriteItem.h"
#include "GraphicItem/FilledItem.h"
#include "GraphicItem/SlicedItem.h"
#include "AnimateSDK/app/ApplicationFCMPublicIDs.h"

namespace sc {
	namespace Adobe {
		class SCWriter;
		struct Sprite;

		class SCShapeWriter : public SharedShapeWriter {
		public:
			SCShapeWriter(SCWriter& writer, SymbolContext& symbol) : m_writer(writer), m_symbol(symbol), m_group(symbol) {};
			virtual ~SCShapeWriter() = default;

			const float RasterizationResolution = 2.0f;

		public:
			virtual void AddGraphic(const SpriteElement& item, const DOM::Utils::MATRIX2D& matrix);

			virtual void AddFilledElement(const FilledElement& shape);

			virtual void AddSlicedElements(const std::vector<FilledElement>& elements, const DOM::Utils::RECT& guides);

			virtual bool Finalize(uint16_t id, bool required);

		public:
			void AddTriangulatedRegion(
				const FilledElementPath& contour,
				const std::vector<FilledElementPath>& holes,
				const DOM::Utils::COLOR& color
			);

			void AddRasterizedRegion(
				const FilledElementRegion& region,
				cv::Mat& canvas,
				DOM::Utils::RECT bound,
				Point<int32_t> offset = { 0, 0 }
			);

			void AddRasterizedRegion(
				const FilledElementRegion& region
			);

		public:
			void AddFilledShapeRegion(const FilledElementRegion& region, const DOM::Utils::MATRIX2D& matrix);

			bool IsValidFilledShapeRegion(const FilledElementRegion& region);

			bool IsComplexShapeRegion(const FilledElementRegion& region);

		public:
			static void RoundDomRectangle(DOM::Utils::RECT& rect);

		private:
			SCWriter& m_writer;
			SymbolContext& m_symbol;

			GraphicGroup m_group;
		};
	}
}