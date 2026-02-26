#pragma once

#include "AnimatePublisher.h"
#include "core/image/raw_image.h"
#include "core/math/point.h"

#include <include/codec/SkPngDecoder.h>
#include <include/core/SkBitmap.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkData.h>
#include <include/core/SkImage.h>
#include <include/core/SkPathBuilder.h>
#include <include/core/SkStream.h>
#include <include/core/SkSurface.h>
#include <include/effects/SkGradient.h>
#include <include/pathops/SkPathOps.h>
#include <include/core/SkPathUtils.h>
#include <include/core/SkPathMeasure.h>
#include <src/core/SkPathPriv.h>

namespace sc::Adobe {
    using RawShape = Animate::Publisher::FilledElementRegion;
    using VectorRegion = Animate::Publisher::FilledElementRegion;
    using VectorSegment = Animate::Publisher::FilledElementPathSegment;
    using VectorLineSegment = Animate::Publisher::FilledElementPathLineSegment;
    using VectorCubicSegment = Animate::Publisher::FilledElementPathCubicSegment;
    using VectorQuadSegment = Animate::Publisher::FilledElementPathQuadSegment;
    using VectorMatrix = Animate::DOM::Utils::MATRIX2D;
    using VectorBound = Animate::DOM::Utils::RECT;

    class SCWriter;

    struct SkShape {
    public:
        RawShape::ShapeType type;
        RawShape::FillStyle style;

        SkPath contour;
        std::vector<SkPath> holes;

        SkPath& GetPath() const;
        void SetPath(const SkPath& path) const;

    private:
        mutable wk::Ref<SkPath> m_path;
    };

    class VectorRasterizer {
    public:
        VectorRasterizer(SCWriter& writer) :
            m_writer(writer) {}

    public:
        void Add(const RawShape& shape, const VectorMatrix& matrix);

        bool Empty() const;
        bool GetImage(wk::RawImageRef& image, VectorMatrix& matrix, float resolution);

    private:
        static void RoundBound(SkRect& rect);
        static void CreatePath(const Animate::Publisher::FilledElementPath& path, SkPathBuilder& contour);

    private:
        /// <summary>
        /// Draw region in active canvas context
        /// </summary>
        /// <param name="region">Region itself</param>
        /// <param name="offset">Region offset</param>
        /// <param name="resolution">Draw resolution</param>
        void DrawRegion(const SkShape& region, wk::PointF offset, float resolution = 1.f);

        /// <summary>
        /// Create canvas context by given bound
        /// </summary>
        /// <param name="bound"></param>
        ///
        bool CreateCanvas(const SkRect& bound, float resolution);

        /// <summary>
        /// Destroy canvas context and flush drawing
        /// </summary>
        void ReleaseCanvas();

        void FillHoles();

    private:
        SCWriter& m_writer;

        std::vector<SkShape> m_queue;

        wk::RawImageRef m_image;
        sk_sp<SkSurface> m_canvas;
        SkCanvas* m_draw = nullptr;
    };
}