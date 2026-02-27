#include "VectorRasterizer.h"

#include "Writer.h"

template <class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

#if WK_DEBUG

    #include "include/encode/SkPngEncoder.h"
static void DebugDrawPathAuto(const SkPath& originalPath,
                              const char* filename = "debug.png",
                              bool drawStrokeOverlay = false,
                              float padding = 20.0f) {
    if (originalPath.isEmpty())
        return;

    SkRect bounds = originalPath.getBounds();

    if (bounds.isEmpty())
        return;

    const int width = static_cast<int>(std::ceil(bounds.width() + padding * 2.0f));
    const int height = static_cast<int>(std::ceil(bounds.height() + padding * 2.0f));

    SkImageInfo info =
        SkImageInfo::Make(width, height, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);

    sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
    if (!surface)
        return;

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);

    SkPathBuilder builder;
    builder.addPath(originalPath);

    SkMatrix translate;
    translate.setTranslate(padding - bounds.left(), padding - bounds.top());

    builder.transform(translate);
    SkPath path = builder.detach();

    SkPaint fillPaint;
    fillPaint.setAntiAlias(true);
    fillPaint.setStyle(SkPaint::kFill_Style);
    fillPaint.setColor(SkColorSetARGB(255, 100, 150, 255));

    canvas->drawPath(path, fillPaint);

    if (drawStrokeOverlay) {
        SkPaint strokePaint;
        strokePaint.setAntiAlias(true);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        strokePaint.setStrokeWidth(2.0f);
        strokePaint.setColor(SK_ColorRED);

        canvas->drawPath(path, strokePaint);
    }

    sk_sp<SkImage> image = surface->makeImageSnapshot();
    if (!image)
        return;

    SkPixmap src;
    if (!image->peekPixels(&src))
        return;

    sk_sp<SkData> png = SkPngEncoder::Encode(src, {});
    if (!png)
        return;

    SkFILEWStream out(filename);
    out.write(png->data(), png->size());
    out.flush();
}
#endif

static bool IsPathCCW(const SkPath& path) {
    SkPathMeasure meas(path, true);

    SkPoint prevPt, firstPt;
    bool first = true;
    float area = 0.0f;
    float length = meas.getLength();
    constexpr int steps = 100;

    if (length == 0.0f)
        return false;

    for (int i = 0; i <= steps; ++i) {
        SkPoint pt;
        float distance = length * (float(i) / steps);
        if (!meas.getPosTan(distance, &pt, nullptr))
            continue;

        if (first) {
            firstPt = pt;
            prevPt = pt;
            first = false;
        } else {
            area += (prevPt.fX * pt.fY) - (pt.fX * prevPt.fY);
            prevPt = pt;
        }
    }

    area += (prevPt.fX * firstPt.fY) - (firstPt.fX * prevPt.fY);
    area *= 0.5f;

    return area < 0.0f;
}

namespace sc::Adobe {
    SkPath& SkShape::GetPath(const SkMatrix* matrix) const {
        if (m_path)
            return *m_path;

        SkPathBuilder builder;
        for (auto& hole : holes) {
            const float inset = 0.5f;

            SkPaint strokePaint;
            strokePaint.setStyle(SkPaint::kStroke_Style);
            strokePaint.setStrokeWidth(inset * 2.0f); // 0.5px inset -> 1px stroke
            strokePaint.setStrokeJoin(SkPaint::kMiter_Join);
            strokePaint.setStrokeMiter(4.0f);
            strokePaint.setAntiAlias(true);

            SkPathBuilder stroke_builder;
            SkPath inset_hole_diff;
            SkPath inset_hole;

            bool inset_success = false;
            inset_success = skpathutils::FillPathWithPaint(hole, strokePaint, &stroke_builder);
            if (inset_success) {
                SkPath strokePath = stroke_builder.detach();

                inset_success = Op(hole, strokePath, kDifference_SkPathOp, &inset_hole_diff);
                if (inset_success) {
                    if (IsPathCCW(inset_hole_diff)) {
                        SkPathBuilder inset_hole_builder;
                        SkPathPriv::ReverseAddPath(&inset_hole_builder, inset_hole_diff);
                        inset_hole = inset_hole_builder.detach();
                    } else {
                        inset_hole = inset_hole_diff;
                    }

                    inset_hole.setFillType(SkPathFillType::kWinding);
                }
            }

            builder.addPath(inset_success ? inset_hole : hole);
        }
        builder.addPath(contour);

        m_path = wk::CreateRef<SkPath>(builder.detach(matrix));
        return *m_path;
    }

    void VectorRasterizer::RoundBound(SkRect& rect) {
        rect.setLTRB(std::round(rect.left()),
                     std::round(rect.top()),
                     std::round(rect.right()),
                     std::round(rect.bottom()));
    }

    void VectorRasterizer::CreatePath(const Animate::Publisher::FilledElementPath& path, SkPathBuilder& contour) {
        uint8_t inited = false;
        for (size_t i = 0; path.Count() > i; i++) {
            const VectorSegment& segment = path.GetSegment(i);

            switch (segment.SegmentType()) {
                case VectorSegment::Type::Line: {
                    const auto& seg = (const VectorLineSegment&) segment;

                    if (!inited++) {
                        contour.moveTo(seg.begin.x, seg.begin.y);
                    }

                    contour.lineTo(seg.end.x, seg.end.y);
                } break;
                case VectorSegment::Type::Cubic: {
                    const auto& seg = (const VectorCubicSegment&) segment;

                    if (!inited++) {
                        contour.moveTo(seg.begin.x, seg.begin.y);
                    }

                    contour.cubicTo(seg.control_l.x,
                                    seg.control_l.y,
                                    seg.control_r.x,
                                    seg.control_r.y,
                                    seg.end.x,
                                    seg.end.y);
                } break;
                case VectorSegment::Type::Quad: {
                    const auto& seg = (const VectorQuadSegment&) segment;

                    if (!inited++) {
                        contour.moveTo(seg.begin.x, seg.begin.y);
                    }

                    contour.quadTo(seg.control.x, seg.control.y, seg.end.x, seg.end.y);
                } break;
                default:
                    break;
            }
        }

        contour.close();
    }

    void VectorRasterizer::Add(const RawShape& raw, const VectorMatrix& matrix) {
        SkMatrix path_matrix =
            SkMatrix::MakeAll(matrix.a, matrix.c, matrix.tx, matrix.b, matrix.d, matrix.ty, 0.f, 0.f, 1.f);

        SkShape& shape = m_queue.emplace_back();
        shape.type = raw.type;
        shape.style = raw.style;

        // Convert all holes to paths
        {
            for (const auto& hole : raw.holes) {
                SkPathBuilder builder;
                VectorRasterizer::CreatePath(hole, builder);
                builder.transform(path_matrix);

                shape.holes.emplace_back(builder.detach());
            }
        }

        // Convert path itself then
        {
            SkPathBuilder builder;
            VectorRasterizer::CreatePath(raw.contour, builder);
            builder.transform(path_matrix);
            shape.contour = builder.detach();
        }
    }

    bool VectorRasterizer::Empty() const {
        return m_queue.empty();
    }

    bool VectorRasterizer::GetImage(wk::RawImageRef& image, VectorMatrix& matrix, float resolution) {
        // FillHoles();

        SkRect bound;
        for (const auto& region : m_queue) {
            SkRect local_bound = region.contour.getBounds();
            bound.join(local_bound);
        }
        bound.sort();

        wk::PointF shape_offset;
        shape_offset.x = bound.x();
        shape_offset.y = bound.y();

        VectorRasterizer::RoundBound(bound);
        if (!CreateCanvas(bound, resolution))
            return false;

        for (const auto& region : m_queue) {
            SkRect region_bound = region.contour.getBounds();

            wk::PointF region_offset;
            region_offset.x = bound.top();
            region_offset.y = bound.left();

            region_offset.x = -region_offset.x + (region_offset.x - shape_offset.x);
            region_offset.y = -region_offset.y + (region_offset.y - shape_offset.y);

            DrawRegion(region, region_offset, resolution);
        }

        ReleaseCanvas();
        image = std::move(m_image);
        matrix.a = 1.f / resolution;
        matrix.d = 1.f / resolution;
        matrix.tx = shape_offset.x / matrix.a;
        matrix.ty = shape_offset.y / matrix.d;

        m_queue.clear();
        return true;
    }

    bool VectorRasterizer::CreateCanvas(const SkRect& bound, float resolution) {
        uint16_t width = std::ceil(bound.width()) * resolution;
        uint16_t height = std::ceil(bound.height()) * resolution;
        if (width == 0 || height == 0)
            return false;

        m_image =
            wk::CreateRef<wk::RawImage>(width, height, wk::Image::PixelDepth::RGBA8, wk::Image::ColorSpace::Linear);

        sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();
        SkImageInfo info = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType, colorSpace);
        size_t rowBytes = info.minRowBytes();
        m_canvas = SkSurfaces::WrapPixels(info, m_image->data(), rowBytes);
        m_draw = m_canvas->getCanvas();

        return true;
    }

    void VectorRasterizer::ReleaseCanvas() {
        m_draw = nullptr;
        m_canvas.reset();
    }

    void VectorRasterizer::CreateFillPaint(const VectorRegion::SolidFill& fill, SkPaint& paint) const {
        paint.setColor(SkColorSetARGB(fill.color.alpha, fill.color.red, fill.color.green, fill.color.blue));
        paint.setStyle(SkPaint::kFill_Style);
    }

    void VectorRasterizer::CreateFillPaint(const VectorRegion::BitmapFill& fill,
                                           SkPaint& paint,
                                           const SkMatrix& resolution_matrix) const {
        fill.bitmap.ExportImage(m_writer.sprite_temp_path);
        sk_sp<SkData> data = SkData::MakeFromFileName(m_writer.sprite_temp_path.string().c_str());
        if (!data)
            return;

        SkCodec::Result decode_result;
        auto codec = SkPngDecoder::Decode(data, &decode_result);
        if (decode_result != SkCodec::kSuccess)
            return;

        auto [bitmap, image_result] = codec->getImage();
        if (image_result != SkCodec::kSuccess)
            return;

        SkTileMode mode = fill.is_clipped ? SkTileMode::kRepeat : SkTileMode::kDecal;
        SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kNone);

        auto matrix = fill.bitmap.Transformation();
        SkMatrix pattern_matrix = SkMatrix::MakeAll(matrix.a / Animate::DOM::TWIPS_PER_PIXEL,
                                                    matrix.c / Animate::DOM::TWIPS_PER_PIXEL,
                                                    matrix.tx,
                                                    matrix.b / Animate::DOM::TWIPS_PER_PIXEL,
                                                    matrix.d / Animate::DOM::TWIPS_PER_PIXEL,
                                                    matrix.ty,
                                                    0,
                                                    0,
                                                    1.f);

        SkMatrix scaled_matrix = SkMatrix::Concat(resolution_matrix, pattern_matrix);

        paint.setShader(bitmap->makeShader(mode, mode, sampling, scaled_matrix));
    }

    void VectorRasterizer::CreateFillPaint(const VectorRegion::GradientFill& fill,
                                           SkPaint& paint,
                                           wk::PointF offset,
                                           const SkMatrix& scale_matrix) const {
        using namespace Animate::DOM;
        sk_sp<SkShader> shader;

        SkTileMode tile_mode = SkTileMode::kDecal;
        switch (fill.spread) {
            case FillStyle::GradientSpread::GRADIENT_SPREAD_EXTEND:
                tile_mode = SkTileMode::kClamp;
                break;
            case FillStyle::GradientSpread::GRADIENT_SPREAD_REFLECT:
                tile_mode = SkTileMode::kMirror;
                break;
            case FillStyle::GradientSpread::GRADIENT_SPREAD_REPEAT:
                tile_mode = SkTileMode::kRepeat;
                break;
            default:
                break;
        }

        VectorMatrix matrix = fill.matrix;
        SkMatrix pattern_matrix = SkMatrix::MakeAll(matrix.a,
                                                    matrix.c,
                                                    matrix.tx + offset.x,
                                                    matrix.b,
                                                    matrix.d,
                                                    matrix.ty + offset.y,
                                                    0.f,
                                                    0.f,
                                                    1.f);

        pattern_matrix = SkMatrix::Concat(scale_matrix, pattern_matrix);

        std::vector<SkColor4f> colors;
        colors.reserve(fill.points.size());

        std::vector<SkScalar> positions;
        positions.reserve(fill.points.size());

        for (auto& point : fill.points) {
            colors.emplace_back(SkColor4f::FromColor(
                SkColorSetARGB(point.color.alpha, point.color.red, point.color.green, point.color.blue)));

            positions.push_back((float) point.pos / 0xFF);
        };

        SkGradient::Colors gradient_colors(SkSpan<const SkColor4f>(colors), SkSpan<const float>(positions), tile_mode);
        SkGradient::Interpolation interp;
        SkGradient gradient(gradient_colors, interp);

        switch (fill.type) {
            case VectorRegion::GradientFill::FillType::Linear: {
                const SkPoint points[2] = {SkPoint::Make(-0x333, 0), SkPoint::Make(0x333, 0)};
                shader = SkShaders::LinearGradient(points, gradient, &pattern_matrix);
            } break;
            case VectorRegion::GradientFill::FillType::Radial: {
                shader = SkShaders::TwoPointConicalGradient(SkPoint::Make(fill.focal_point, 0.f),
                                                            0.0f,
                                                            SkPoint::Make(0.f, 0.f),
                                                            0x333,
                                                            gradient,
                                                            &pattern_matrix);
            } break;
            default:
                return;
        }

        paint.setShader(shader);
    }

    void VectorRasterizer::DrawRegion(const SkShape& region, wk::PointF offset, float resolution) {
        using namespace Animate::DOM;

        SkMatrix scale_matrix = SkMatrix::Scale(resolution, resolution);
        SkMatrix offset_matrix = SkMatrix::Translate(offset.x, offset.y);
        SkMatrix resolution_matrix = SkMatrix::Concat(scale_matrix, offset_matrix);

        // Creating fill style
        SkPaint paint;
        paint.setAntiAlias(true);
        std::visit(Overloaded {[&](VectorRegion::SolidFill fill) { CreateFillPaint(fill, paint); },
                               [&](const VectorRegion::BitmapFill& fill) {
                                   CreateFillPaint(fill, paint, resolution_matrix);
                               },
                               [&](const VectorRegion::GradientFill& fill) {
                                   CreateFillPaint(fill, paint, offset, scale_matrix);
                               }},
                   region.style);

        m_draw->drawPath(region.GetPath(&resolution_matrix), paint);
    }
}