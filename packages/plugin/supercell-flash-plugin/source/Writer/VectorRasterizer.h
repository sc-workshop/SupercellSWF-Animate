#pragma once

#include "AnimatePublisher.h"
#include "core/image/raw_image.h"
#include "core/math/point.h"

#include <include/core/SkBitmap.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkData.h>
#include <include/core/SkImage.h>
#include <include/core/SkPathBuilder.h>
#include <include/core/SkStream.h>
#include <include/core/SkSurface.h>
#include <include/codec/SkPngDecoder.h>
#include <include/effects/SkGradient.h>

namespace sc::Adobe {
    using VectorShape = Animate::Publisher::FilledElementRegion;
    using VectorRegion = Animate::Publisher::FilledElementRegion;
    using VectorSegment = Animate::Publisher::FilledElementPathSegment;
    using VectorLineSegment = Animate::Publisher::FilledElementPathLineSegment;
    using VectorCubicSegment = Animate::Publisher::FilledElementPathCubicSegment;
    using VectorQuadSegment = Animate::Publisher::FilledElementPathQuadSegment;
    using VectorMatrix = Animate::DOM::Utils::MATRIX2D;
    using VectorBound = Animate::DOM::Utils::RECT;

    class SCWriter;

    class VectorRasterizer {
    public:
        VectorRasterizer(SCWriter& writer) :
            m_writer(writer) {}

    public:
        void Add(const VectorShape& shape, const VectorMatrix& matrix);

        bool Empty() const;
        bool GetImage(wk::RawImageRef& image, VectorMatrix& matrix, float resolution);

    private:
        static void CreateImage(wk::RawImageRef& image, sk_sp<SkSurface>& result, bool premultiply);
        static void RoundBound(Animate::DOM::Utils::RECT& rect);
        static void CreatePath(const Animate::Publisher::FilledElementPath& path,
                               SkPathBuilder& contour,
                               float resolution = 1.f);

    private:
        /// <summary>
        /// Draw region in active canvas context
        /// </summary>
        /// <param name="region">Region itself</param>
        /// <param name="offset">Region offset</param>
        /// <param name="resolution">Draw resolution</param>
        void DrawRegion(const Animate::Publisher::FilledElementRegion& region,
                        wk::PointF offset,
                        float resolution = 1.f);

        /// <summary>
        /// Create canvas context by given bound
        /// </summary>
        /// <param name="bound"></param>
        ///
        bool CreateCanvas(const Animate::DOM::Utils::RECT bound, float resolution);

        /// <summary>
        /// Destroy canvas context and flush drawing
        /// </summary>
        void ReleaseCanvas();

    private:
        SCWriter& m_writer;

        std::vector<VectorShape> m_queue;

        wk::RawImageRef m_image;
        sk_sp<SkSurface> m_canvas;
        SkCanvas* m_draw = nullptr;
    };
}