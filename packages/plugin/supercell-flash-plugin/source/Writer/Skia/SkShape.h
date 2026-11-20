#pragma once

#include "Skia.h"
#include "animate/generator/TimelineBuilder/FrameElements/FilledElement.h"

namespace sc::Adobe {
    class SkShape {
    public:
        Animate::Publisher::FilledElementRegion::FillStyle style;

        SkPath contour;
        std::vector<SkPath> holes;

        SkPath& GetPath(const SkMatrix* matrix = nullptr) const;

    private:
        mutable wk::Ref<SkPath> m_path;
    };
}