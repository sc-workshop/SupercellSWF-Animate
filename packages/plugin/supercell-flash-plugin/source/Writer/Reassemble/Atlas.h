#pragma once

#include "Module/Module.h"
#include "atlas_generator/Generator.h"
#include "core/geometry/intersect.hpp"
#include "flash/flash.h"

using namespace wk;
namespace sc::flash {
    constexpr float cord_min = std::numeric_limits<float>::min();
    constexpr float cord_max = std::numeric_limits<float>::max();

    void getSprite(RawImageRef& texture,
                   AtlasGenerator::RectUV bound,
                   const std::vector<AtlasGenerator::PointUV>& points,
                   RawImageRef& result);
    bool isSolid9Slice(const Shape& shape);
    ShapeDrawBitmapCommand createProxy9Slice(const Shape& shape);

    void repackAtlas(SupercellSWF& swf);
    void removeUnusedTextures(SupercellSWF& swf);
}