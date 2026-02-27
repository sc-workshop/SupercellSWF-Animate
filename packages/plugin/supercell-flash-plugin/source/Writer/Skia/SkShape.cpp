#include "SkShape.h"
#include "SkiaUtils.hpp"

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

}