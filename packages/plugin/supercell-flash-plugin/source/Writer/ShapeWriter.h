#pragma once

#include "AnimatePublisher.h"
#include "core/math/point.h"
#include "core/memory/ref.h"
#include "core/image/raw_image.h"

#include <blend2d/blend2d.h>

namespace sc {
	namespace Adobe {
		class SCWriter;

		struct RasterizingContext
		{
			wk::RawImageRef image;
			BLImage canvas_image;
			BLContext ctx;
		};

		class SCShapeWriter : public Animate::Publisher::SharedShapeWriter {
		public:
			SCShapeWriter(SCWriter& writer, Animate::Publisher::SymbolContext& symbol) :
				Animate::Publisher::SharedShapeWriter(symbol),
				m_writer(writer) {};
			virtual ~SCShapeWriter() = default;

			static inline const float RasterizationResolution = 2.f;

		public:
			virtual void AddGraphic(const Animate::Publisher::BitmapElement& item);
			virtual void AddFilledElement(const Animate::Publisher::FilledElement& shape);
			virtual void AddSlicedElements(const Animate::Publisher::Slice9Element& slice);

			virtual bool Finalize(ResourceReference reference, bool required, bool new_symbol);

		protected:
			virtual std::size_t GenerateHash() const;

		public:
			void AddTriangulatedRegion(
				const Animate::Publisher::FilledElementPath& contour,
				const std::vector<Animate::Publisher::FilledElementPath>& holes,
				const Animate::DOM::Utils::MATRIX2D& matrix,
				const Animate::DOM::Utils::COLOR& color
			);

			void AddRasterizedRegion(
				const Animate::Publisher::FilledElementRegion& region,
				const Animate::DOM::Utils::MATRIX2D& matrix,
				float resolution = 1.f
			);

		public:
			void AddFilledShapeRegion(const Animate::Publisher::FilledElementRegion& region, const Animate::DOM::Utils::MATRIX2D& matrix);
			bool IsValidFilledShapeRegion(const Animate::Publisher::FilledElementRegion& region);
			bool IsComplexShapeRegion(const Animate::Publisher::FilledElementRegion& region);

		public:
			static void RoundDomRectangle(Animate::DOM::Utils::RECT& rect);

		private: // canvas releated functions

			/// <summary>
			/// Create canvas context by given bound
			/// </summary>
			/// <param name="bound"></param>
			bool CreateCanvas(const Animate::DOM::Utils::RECT bound, float resolution);

			/// <summary>
			/// Destroy canvas context and flush drawing
			/// </summary>
			void ReleaseCanvas();

			/// <summary>
			/// Draw region on existing image
			/// </summary>
			/// <param name="image">Draw surface</param>
			/// <param name="region">Region itself</param>
			/// <param name="offset">Surface offset</param>
			/// <param name="resolution">Draw resolution</param>
			void DrawRegionTo(const wk::RawImageRef image, const Animate::Publisher::FilledElementRegion& region, wk::Point offset, float resolution = 1.f);

			/// <summary>
			/// Draw region in active canvas context
			/// </summary>
			/// <param name="region">Region itself</param>
			/// <param name="offset">Region offset</param>
			/// <param name="resolution">Draw resolution</param>
			void DrawRegion(const Animate::Publisher::FilledElementRegion& region, wk::PointF offset, float resolution = 1.f);

			/// <summary>
			/// Draw region
			/// </summary>
			/// <param name="region">Region itself</param>
			/// <param name="resolution">Draw resolution</param>
			/// <param name="result">Result image</param>
			/// <param name="offset">Result region offset</param>
			bool DrawRegion(const Animate::Publisher::FilledElementRegion& region, float resolution, wk::RawImageRef& result, wk::Point& offset);

			static void CreatePath(const Animate::Publisher::FilledElementPath& path, wk::PointF offset, BLPath& contour, float resolution = 1.f);

			static void RoundRegion(Animate::Publisher::FilledElementRegion& path);
			static void RoundPath(Animate::Publisher::FilledElementPath& path);

			static void CreateImage(wk::RawImageRef& image, BLImage& result, bool premultiply);

		private:
			SCWriter& m_writer;
			Animate::Publisher::StaticElementsGroup m_group;
			wk::Unique<RasterizingContext> canvas;
		};
	}
}
