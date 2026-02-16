#pragma once

#include "Writer/GraphicItem/FilledItem.h"
#include "Writer/GraphicItem/GraphicItem.h"
#include "Writer/GraphicItem/SlicedItem.h"
#include "Writer/GraphicItem/SpriteItem.h"
#include "atlas_generator/Generator.h"
#include "flash/flash.h"

namespace sc::Adobe {
    class SCWriter;

    class AtlasWriter {
    public:
        AtlasWriter(SCWriter& writer);

    public:
        void ProcessDrawCommand(flash::ShapeDrawBitmapCommand& command,
                                wk::AtlasGenerator::Item::Transformation<int32_t>& transform,
                                GraphicItem& item);

        template <typename T>
        void ProcessVertices(flash::Shape& shape,
                             const wk::AtlasGenerator::Container<wk::AtlasGenerator::Vertex_t<T>>& vertices,
                             wk::AtlasGenerator::Item& atlas_item,
                             GraphicItem& item) {
            using namespace wk;
            using namespace AtlasGenerator;

            flash::ShapeDrawBitmapCommand& shape_command = shape.commands.emplace_back();
            shape_command.texture_index = (uint32_t) atlas_item.texture_index + texture_offset;

            for (const auto& vertex : vertices) {
                auto& shape_vertex = shape_command.vertices.emplace_back();

                shape_vertex.x = vertex.xy.x;
                shape_vertex.y = vertex.xy.y;
                shape_vertex.u = vertex.uv.u;
                shape_vertex.v = vertex.uv.v;
            }

            ProcessDrawCommand(shape_command, atlas_item.transform, item);
        }

        void ProcessSpriteItem(flash::Shape& shape, wk::AtlasGenerator::Item& atlas_item, BitmapItem& sprite_item);

        void ProcessFilledItem(flash::Shape& shape, wk::AtlasGenerator::Item& atlas_item, FilledItem& filled_item);

        void ProcessSlicedItem(flash::Shape& shape, wk::AtlasGenerator::Item& atlas_item, SlicedItem& sprite_item);

        void WriteAtlas();

    public:
        uint32_t texture_offset = 0;

    private:
        SCWriter& m_writer;
    };
}
