#pragma once

#include "AnimatePublisher.h"
#include "GraphicItem/GraphicGroup.h"

#include "core/math/point.h"

namespace sc {
	namespace Adobe {
		class SCWriter;

		class SCShapeWriter : public Animate::Publisher::SharedShapeWriter{
		public:
			SCShapeWriter(SCWriter& writer, Animate::Publisher::SymbolContext& symbol) : m_writer(writer), m_symbol(symbol), m_group(symbol) {};
			virtual ~SCShapeWriter() = default;

			const float RasterizationResolution = 2.0f;

		public:
			virtual void AddGraphic(const Animate::Publisher::SpriteElement& item, const Animate::DOM::Utils::MATRIX2D& matrix);

			virtual void AddFilledElement(const Animate::Publisher::FilledElement& shape);

			virtual void AddSlicedElements(const std::vector<Animate::Publisher::FilledElement>& elements, const Animate::DOM::Utils::RECT& guides);

			virtual bool Finalize(uint16_t id, bool required);

		public:
			void AddTriangulatedRegion(
				const Animate::Publisher::FilledElementPath& contour,
				const std::vector<Animate::Publisher::FilledElementPath>& holes,
				const Animate::DOM::Utils::COLOR& color
			);

			void AddRasterizedRegion(
				const Animate::Publisher::FilledElementRegion& region,
				cv::Mat& canvas,
				Animate::DOM::Utils::RECT bound,
				wk::Point offset = { 0, 0 }
			);

			void AddRasterizedRegion(
				const Animate::Publisher::FilledElementRegion& region
			);

		public:
			void AddFilledShapeRegion(const Animate::Publisher::FilledElementRegion& region, const Animate::DOM::Utils::MATRIX2D& matrix);

			bool IsValidFilledShapeRegion(const Animate::Publisher::FilledElementRegion& region);

			bool IsComplexShapeRegion(const Animate::Publisher::FilledElementRegion& region);

		public:
			static void RoundDomRectangle(Animate::DOM::Utils::RECT& rect);

		private:
			SCWriter& m_writer;
			Animate::Publisher::SymbolContext& m_symbol;

			GraphicGroup m_group;
		};
	}
}