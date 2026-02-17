#include "ShapeWriter.h"

#include "Module/Module.h"
#include "Writer.h"
#include "atlas_generator/Item/Item.h"
#include "core/hashing/hash.h"
#include "core/hashing/ncrypto/xxhash.h"

#include <CDT.h>

using namespace Animate::Publisher;

namespace sc::Adobe {
    void SCShapeWriter::AddGraphic(const Animate::Publisher::BitmapElement& item) {
        wk::RawImageRef image = m_writer.GetBitmap(item);

        m_group.AddElement<BitmapItem>(m_symbol, image, item.Transformation());
    }

    void SCShapeWriter::AddFilledElement(const Animate::Publisher::FilledElement& shape) {
        for (const auto& region : shape.fill) {
            AddFilledShapeRegion(region, shape.Transformation());
        }

        for (const auto& region : shape.stroke) {
            AddFilledShapeRegion(region, shape.Transformation());
        }
    }

    void SCShapeWriter::AddContourRegion(const Animate::Publisher::FilledElementPath& path,
                                         const Animate::DOM::Utils::MATRIX2D& matrix,
                                         const Animate::DOM::Utils::COLOR& color) {
        std::vector<Point2D> points;
        path.Rasterize(points);

        std::vector<FilledItemContour> contour;
        if (m_guides) {
            using namespace wk::AtlasGenerator;
            contour.reserve(9);

            // Pretransform vertices
            Container<VertexF> vertices;
            for (const Point2D& point : points) {
                // Also need to multiply by rasterization resolotion for guides
                vertices.emplace_back((matrix.a * point.x + matrix.b * point.y + matrix.tx) *
                                          SCShapeWriter::RasterizationResolution,
                                      (matrix.c * point.x + matrix.d * point.y + matrix.ty) *
                                          SCShapeWriter::RasterizationResolution,
                                      0,
                                      0);
            }

            // Generate 9slice contours
            Container<Container<wk::AtlasGenerator::VertexF>> regions;
            Item::Generate9Slice(SlicedItem::RoundScalingGrid(*m_guides), regions, vertices);

            for (auto& region : regions) {
                std::vector<Point2D> contour_points;
                for (auto& point : region) {
                    contour_points.emplace_back(point.xy.x, point.xy.y);
                }

                contour.emplace_back(contour_points);
            }

            Animate::DOM::Utils::MATRIX2D regions_matrix = {1.0f / SCShapeWriter::RasterizationResolution,
                                                            0.f,
                                                            0.f,
                                                            1.0f / SCShapeWriter::RasterizationResolution,
                                                            0.f,
                                                            0.f};

            m_group.AddElement<FilledItem>(m_symbol, contour, color, regions_matrix);
        } else {
            contour = {FilledItemContour(points)};
            m_group.AddElement<FilledItem>(m_symbol, contour, color, matrix);
        }
    }

    void SCShapeWriter::AddTriangulatedRegion(const Animate::Publisher::FilledElementPath& contour,
                                              const std::vector<Animate::Publisher::FilledElementPath>& holes,
                                              const Animate::DOM::Utils::MATRIX2D& matrix,
                                              const Animate::DOM::Utils::COLOR& color) {
        using namespace CDT;

        Triangulation<float> cdt(VertexInsertionOrder::Auto, IntersectingConstraintEdges::TryResolve, 0);

        std::vector<V2d<float>> vertices;
        std::vector<Edge> edges;

        {
            std::vector<Animate::Publisher::Point2D> points;
            contour.Rasterize(points);

            for (Animate::Publisher::Point2D& point : points) {
                vertices.push_back({point.x, point.y});
            }
        }

        // Contour
        for (uint32_t i = 0; vertices.size() > i; i++) {
            uint32_t secondIndex = i + 1;
            if (secondIndex >= vertices.size()) {
                secondIndex = 0;
            }
            edges.push_back(Edge(i, secondIndex));
        }

        // Holes
        for (const auto& hole : holes) {
            std::vector<Animate::Publisher::Point2D> points;
            hole.Rasterize(points);

            uint32_t vertexSize = (uint32_t) vertices.size();
            for (uint32_t i = 0; points.size() > i; i++) {
                uint32_t secondIndex = vertexSize + i + 1;
                if (secondIndex >= points.size() + vertices.size()) {
                    secondIndex = vertexSize;
                }
                edges.push_back(Edge(vertexSize + i, secondIndex));
            }

            for (const auto& point : points) {
                vertices.push_back({point.x, point.y});
            }
        }

        RemoveDuplicatesAndRemapEdges(vertices, edges);

        cdt.insertVertices(vertices);
        cdt.insertEdges(edges);

        cdt.eraseOuterTrianglesAndHoles();

        std::vector<FilledItemContour> contours;

        for (const Triangle& triangle : cdt.triangles) {
            auto& point1 = cdt.vertices[triangle.vertices[0]];
            auto& point2 = cdt.vertices[triangle.vertices[1]];
            auto& point3 = cdt.vertices[triangle.vertices[2]];

            std::vector<Animate::Publisher::Point2D> triangle_shape({
                {point1.x, point1.y},
                {point2.x, point2.y},
                {point3.x, point3.y},
                {point3.x, point3.y},
            });

            contours.emplace_back(triangle_shape);
        }

        m_group.AddElement<FilledItem>(m_symbol, contours, color, matrix);
    }

    void SCShapeWriter::FlushRasterizer() {
        if (m_rasterizer.Empty())
            return;

        bool is_9slice = m_guides.has_value();

        wk::RawImageRef sprite;
        VectorMatrix matrix;
        if (!m_rasterizer.GetImage(sprite, matrix, is_9slice ? SCShapeWriter::RasterizationResolution : 1.0f))
            return;

        if (is_9slice) {
            // Extracting translation values for regions generating
            wk::PointF offset;
            offset.x = matrix.tx;
            offset.y = matrix.ty;

            // Null translation values since get_9slice returns already translated vertices
            matrix.tx = 0;
            matrix.ty = 0;

            m_group.AddElement<SlicedItem>(m_symbol, sprite, matrix, offset, m_guides.value());
        } else {
            m_group.AddElement<BitmapItem>(m_symbol, sprite, matrix, true);
        }
    }

    bool SCShapeWriter::IsComplexShapeRegion(const FilledElementRegion& region) {
        for (size_t i = 0; region.contour.Count() > i; i++) {
            const FilledElementPathSegment& segment = region.contour.GetSegment(i);

            if (segment.SegmentType() != FilledElementPathSegment::Type::Line) {
                return true;
            }
        }

        for (const FilledElementPath& path : region.holes) {
            for (size_t i = 0; path.Count() > i; i++) {
                const FilledElementPathSegment& segment = path.GetSegment(i);

                if (segment.SegmentType() != FilledElementPathSegment::Type::Line) {
                    return true;
                }
            }
        }

        return false;
    }

    bool SCShapeWriter::IsValidFilledShapeRegion(const FilledElementRegion& region) {
        if (region.type == FilledElementRegion::ShapeType::SolidColor) {
            const auto& fill = std::get<FilledElementRegion::SolidFill>(region.style);
            // Skip all regions with zero mask_alpha
            if (fill.color.alpha <= 0) {
                return false;
            }
        } else if (region.type == FilledElementRegion::ShapeType::Bitmap) {
            return true;
        } else if (region.type == FilledElementRegion::ShapeType::GradientColor) {
            return true;
        } else {
            return false;
        }

        return true;
    }

    void SCShapeWriter::AddFilledShapeRegion(const FilledElementRegion& region,
                                             const Animate::DOM::Utils::MATRIX2D& matrix) {
        if (!IsValidFilledShapeRegion(region))
            return;

        bool should_rasterize =
            region.type != FilledElementRegion::ShapeType::SolidColor || IsComplexShapeRegion(region);

        bool is_contour =
            !should_rasterize && region.contour.Count() <= 4 && !(region.contour.Count() > 6) && region.holes.empty();

        bool should_triangulate = !should_rasterize && region.contour.Count() > 4;

        if (should_rasterize)
            return m_rasterizer.Add(region, matrix);

        // Flush all vector elements if any
        FlushRasterizer();

        // Any non-solid color fill will be rasterized, so at this moment
        // we have guarantee that fill style is Solid Color
        const auto& fill = std::get<FilledElementRegion::SolidFill>(region.style);
        if (is_contour) {
            AddContourRegion(region.contour, matrix, fill.color);
        } else if (should_triangulate) {
            AddTriangulatedRegion(region.contour, region.holes, matrix, fill.color);
        }
    }

    void SCShapeWriter::AddSlicedElements(const Animate::Publisher::Slice9Element& slice) {
        // 9Slice sprites are usually very pixelated
        // So we need to scale their resolution by 2
        // But xy coordinates must be remains the same

        const float resolution = SCShapeWriter::RasterizationResolution;
        auto guides = slice.Guides();

        // Create a scaled guide to cut final sprite
        m_guides = {{guides.topLeft.x * resolution, guides.topLeft.y * resolution},
                    {guides.bottomRight.x * resolution, guides.bottomRight.y * resolution}};

        const auto& elements = slice.Elements();
        for (size_t i = 0; elements.Size() > i; i++) {
            StaticElement& element = elements[i];

            if (element.IsFilledArea()) {
                AddFilledElement((FilledElement&) element);
            } else if (element.IsSprite()) {
                AddGraphic((BitmapElement&) element);
            }
        }
    }

    std::size_t SCShapeWriter::GenerateHash() const {
        wk::hash::XxHash code;

        for (size_t i = 0; m_group.Size() > i; i++) {
            const GraphicItem& item = (const GraphicItem&) m_group[i];
            code.update(item);
        }

        return code.digest();
    }

    void SCShapeWriter::PreFinalize() {
        FlushRasterizer();
    }

    bool SCShapeWriter::Finalize(ResourceReference reference, bool required, bool new_symbol) {
        if (!new_symbol)
            return true;

        if (m_group.Size() == 0) {
            if (required) {
                // Small workaround to avoid crashes with empty shapes but keep it required
                flash::MovieClip& movieclip = m_writer.swf.movieclips.emplace_back();
                movieclip.id = reference.GetId();

                return true;
            } else {
                return false;
            }
        }

        flash::Shape& shape = m_writer.swf.shapes.emplace_back();
        shape.id = reference.GetId();

        m_writer.AddGraphicGroup(m_group);

        return true;
    }
}

namespace wk::hash {
    template <>
    struct Hash_t<sc::Adobe::GraphicItem> {
        template <typename T>
        static void update(wk::hash::HashStream<T>& stream, const sc::Adobe::GraphicItem& item) {
            stream.update(item.Transformation2D());
            if (item.IsSprite()) {
                const sc::Adobe::BitmapItem& sprite = (const sc::Adobe::BitmapItem&) item;
                const auto& image = sprite.Image();
                stream.update((const wk::Image&) image);

                if (sprite.Is9Sliced()) {
                    const sc::Adobe::SlicedItem& sliced = (const sc::Adobe::SlicedItem&) item;
                    stream.update(sliced.Guides());
                }
            } else if (item.IsSolidColor()) {
                const sc::Adobe::FilledItem& fill = (const sc::Adobe::FilledItem&) item;

                stream.update(fill.Color());

                for (auto& contour : fill.contours) {
                    for (auto& point : contour.Contour()) {
                        stream.update(point.x);
                        stream.update(point.y);
                    }
                }
            }
        }
    };
}
