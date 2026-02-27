#pragma once

#include "Skia.h"

namespace sc::Adobe {
    static bool IsPathCCW(const SkPath& path) {
        SkPathMeasure meas(path, true);

        SkPoint prevPt = {0, 0};
        SkPoint firstPt = {0, 0};
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
}