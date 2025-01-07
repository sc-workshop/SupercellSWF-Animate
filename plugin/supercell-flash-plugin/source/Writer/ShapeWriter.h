#pragma once

#include "AnimatePublisher.h"

#include "core/math/point.h"

namespace sc {
	namespace Adobe {
		class SCWriter;

		class SCShapeWriter : public Animate::Publisher::SharedShapeWriter {
		public:
			SCShapeWriter(SCWriter& writer, Animate::Publisher::SymbolContext& symbol) :
				Animate::Publisher::SharedShapeWriter(symbol),
				m_writer(writer) {};
			virtual ~SCShapeWriter() = default;

			const float RasterizationResolution = 2.f;

		public:
			virtual void AddGraphic(const Animate::Publisher::BitmapElement& item);
			virtual void AddFilledElement(const Animate::Publisher::FilledElement& shape);
			virtual void AddSlicedElements(const Animate::Publisher::Slice9Element& slice);

			virtual bool Finalize(uint16_t id, bool required, bool new_symbol);

		protected:
			virtual std::size_t GenerateHash() const;

		public:
			void AddTriangulatedRegion(
				const Animate::Publisher::FilledElementPath& contour,
				const std::vector<Animate::Publisher::FilledElementPath>& holes,
				const Animate::DOM::Utils::COLOR& color
			);

			//void AddRasterizedRegion(
			//	const Animate::Publisher::FilledElementRegion& region,
			//	wk::RawImage& canvas,
			//	Animate::DOM::Utils::RECT bound,
			//	wk::Point offset = { 0, 0 }
			//);
			//
			//void AddRasterizedRegion(
			//	const Animate::Publisher::FilledElementRegion& region
			//);

		public:
			void AddFilledShapeRegion(const Animate::Publisher::FilledElementRegion& region, const Animate::DOM::Utils::MATRIX2D& matrix);

			bool IsValidFilledShapeRegion(const Animate::Publisher::FilledElementRegion& region);

			bool IsComplexShapeRegion(const Animate::Publisher::FilledElementRegion& region);

		public:
			static void RoundDomRectangle(Animate::DOM::Utils::RECT& rect);

		private:
			SCWriter& m_writer;
			Animate::Publisher::StaticElementsGroup m_group;
		};
	}
}