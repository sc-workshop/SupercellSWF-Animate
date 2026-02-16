#include "VectorRasterizer.h"

#include "Writer.h"
#include "core/stb/stb.h"

static inline void bl_assert(BLResult result) {
    if (result != BL_SUCCESS) {
        BLResultCode code = (BLResultCode) result;
        assert(code == BL_SUCCESS);
    }
}

namespace sc::Adobe {
    void VectorRasterizer::CreateImage(wk::RawImageRef& image, BLImage& texture, bool premultiply) {
        if (image->depth() != wk::Image::PixelDepth::RGBA8) {
            wk::RawImageRef converted =
                wk::CreateRef<wk::RawImage>(image->width(), image->height(), wk::Image::PixelDepth::RGBA8);

            image->copy(*converted);
            image = converted;
        }

        if (premultiply) {
            for (uint16_t h = 0; image->height() > h; h++) {
                for (uint16_t w = 0; image->width() > w; w++) {
                    wk::ColorRGBA& pixel = image->at<wk::ColorRGBA>(w, h);

                    float alpha = (float) pixel.a / 255.f;

                    pixel.r = (uint8_t) (pixel.r * alpha);
                    pixel.g = (uint8_t) (pixel.g * alpha);
                    pixel.b = (uint8_t) (pixel.b * alpha);
                }
            }
        }

        BLResult result = texture.create_from_data(image->width(),
                                                   image->height(),
                                                   BLFormat::BL_FORMAT_PRGB32,
                                                   image->data(),
                                                   image->pixel_size() * image->width());
        bl_assert(result);
    }

    void VectorRasterizer::RoundBound(Animate::DOM::Utils::RECT& rect) {
        rect.bottomRight.x = std::round(rect.bottomRight.x);
        rect.bottomRight.y = std::round(rect.bottomRight.y);
        rect.topLeft.x = std::round(rect.topLeft.x);
        rect.topLeft.y = std::round(rect.topLeft.y);
    }

    void VectorRasterizer::CreatePath(const Animate::Publisher::FilledElementPath& path,
                                      wk::PointF offset,
                                      BLPath& contour,
                                      float resolution) {
        uint8_t inited = false;
        for (size_t i = 0; path.Count() > i; i++) {
            const VectorSegment& segment = path.GetSegment(i);

            switch (segment.SegmentType()) {
                case VectorSegment::Type::Line: {
                    const auto& seg = (const VectorLineSegment&) segment;

                    if (!inited++) {
                        contour.move_to(seg.begin.x + offset.x, seg.begin.y + offset.y);
                    }

                    contour.line_to(seg.end.x + offset.x, seg.end.y + offset.y);
                }

                break;
                case VectorSegment::Type::Cubic: {
                    const auto& seg = (const VectorCubicSegment&) segment;

                    if (!inited++) {
                        contour.move_to(seg.begin.x + offset.x, seg.begin.y + offset.y);
                    }

                    contour.cubic_to(seg.control_l.x + offset.x,
                                     seg.control_l.y + offset.y,
                                     seg.control_r.x + offset.x,
                                     seg.control_r.y + offset.y,
                                     seg.end.x + offset.x,
                                     seg.end.y + offset.y);
                } break;
                case VectorSegment::Type::Quad: {
                    const auto& seg = (const VectorQuadSegment&) segment;

                    if (!inited++) {
                        contour.move_to(seg.begin.x + offset.x, seg.begin.y + offset.y);
                    }

                    contour.quad_to(seg.control.x + offset.x,
                                    seg.control.y + offset.y,
                                    seg.end.x + offset.x,
                                    seg.end.y + offset.y);
                } break;
                default:
                    break;
            }
        }

        if (resolution != 1.0f) {
            BLMatrix2D matrix(resolution, 0, 0, resolution, 0, 0);
            contour.transform(matrix);
        }
    }

    void VectorRasterizer::Add(const VectorShape& shape, const VectorMatrix& matrix) {
        VectorShape& region = m_queue.emplace_back(shape);
        region.Transform(matrix);
    }

    bool VectorRasterizer::Empty() const {
        return m_queue.empty();
    }

    bool VectorRasterizer::GetImage(wk::RawImageRef& image, VectorMatrix& matrix, float resolution) {
        VectorBound bound;
        for (const VectorShape& region : m_queue) {
            bound = region.Bound(bound);
        }

        wk::PointF shape_offset;
        shape_offset.x = std::min(bound.topLeft.x, bound.bottomRight.x);
        shape_offset.y = std::min(bound.topLeft.y, bound.bottomRight.y);

        VectorRasterizer::RoundBound(bound);
        if (!CreateCanvas(bound, resolution))
            return false;

        for (const VectorShape& region : m_queue) {
            VectorBound region_bound = region.Bound();

            wk::PointF region_offset;
            region_offset.x = std::min(region_bound.topLeft.x, region_bound.bottomRight.x);
            region_offset.y = std::min(region_bound.topLeft.y, region_bound.bottomRight.y);

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

    bool VectorRasterizer::CreateCanvas(const Animate::DOM::Utils::RECT bound, float resolution) {
        uint16_t width = std::ceil(bound.topLeft.x - bound.bottomRight.x) * resolution;
        uint16_t height = std::ceil(bound.topLeft.y - bound.bottomRight.y) * resolution;
        if (width == 0 || height == 0)
            return false;

        m_image =
            wk::CreateRef<wk::RawImage>(width, height, wk::Image::PixelDepth::RGBA8, wk::Image::ColorSpace::Linear);
        VectorRasterizer::CreateImage(m_image, m_canvas, false);

        m_draw = BLContext(m_canvas);
        return true;
    }

    void VectorRasterizer::ReleaseCanvas() {
        bl_assert(m_draw.end());
        m_canvas.reset();
    }

    void VectorRasterizer::DrawRegion(const Animate::Publisher::FilledElementRegion& region,
                                      wk::PointF offset,
                                      float resolution) {
        BLResult result = BL_SUCCESS;
        BLPath region_path;

        // Add holes paths first
        for (const auto& hole : region.holes) {
            BLPath contour;
            VectorRasterizer::CreatePath(hole, offset, contour, resolution);

            // Reverse hole for non zero fill rule
            result = region_path.add_path(contour);
            bl_assert(result);
        }

        // Contour drawing
        {
            BLPath contour;
            VectorRasterizer::CreatePath(region.contour, offset, contour, resolution);
            region_path.add_path(contour);

            
            if (region.type == VectorRegion::ShapeType::SolidColor) {
                const auto& fill = std::get<VectorRegion::SolidFill>(region.style);

                result =
                    m_draw.fill_path(region_path,
                                     BLRgba32(fill.color.blue, fill.color.green, fill.color.red, fill.color.alpha));
            } else if (region.type == VectorRegion::ShapeType::Bitmap) {
                const auto& fill = std::get<VectorRegion::BitmapFill>(region.style);

                fill.bitmap.ExportImage(m_writer.sprite_temp_path);

                wk::RawImageRef image;
                BLImage texture;
                {
                    wk::InputFileStream file(m_writer.sprite_temp_path);
                    wk::stb::load_image(file, image);
                }
                VectorRasterizer::CreateImage(image, texture, true);
                BLPattern pattern(texture);

                auto matrix = fill.bitmap.Transformation();
                matrix.a /= Animate::DOM::TWIPS_PER_PIXEL;
                matrix.b /= Animate::DOM::TWIPS_PER_PIXEL;
                matrix.c /= Animate::DOM::TWIPS_PER_PIXEL;
                matrix.d /= Animate::DOM::TWIPS_PER_PIXEL;
                matrix.tx += offset.x;
                matrix.ty += offset.y;

                matrix.a *= resolution;
                matrix.c *= resolution;

                BLMatrix2D pattern_matrix {matrix.a, matrix.b, matrix.c, matrix.d, matrix.tx, matrix.ty};
                result = pattern.set_transform(pattern_matrix);
                bl_assert(result);

                result = m_draw.fill_path(region_path, pattern);
            }
            bl_assert(result);
        }
    }
}