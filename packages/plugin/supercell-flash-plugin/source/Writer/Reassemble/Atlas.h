#pragma once

#include "Module/Module.h"
#include "Window/Components/StatusComponent.h"
#include "atlas_generator/Generator.h"
#include "core/geometry/intersect.hpp"
#include "flash/flash.h"

using namespace wk;
namespace sc::flash {
    constexpr float cord_min = std::numeric_limits<float>::min();
    constexpr float cord_max = std::numeric_limits<float>::max();

    void get_sprite(RawImageRef& texture,
                    AtlasGenerator::RectUV bound,
                    const std::vector<AtlasGenerator::PointUV>& points,
                    RawImageRef& result);
    bool is_solid_9slice(const Shape& shape);
    ShapeDrawBitmapCommand create_proxy_9slice_command(const Shape& shape);

    void repack_atlas(SupercellSWF& swf);
    void remove_unused_textures(SupercellSWF& swf);
}