#pragma once

#include "Animate/generator/TimelineBuilder/FrameElements/FilledElement.h"
#include "Skia.h"

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