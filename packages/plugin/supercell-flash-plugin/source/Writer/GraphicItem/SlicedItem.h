#pragma once

#include "SpriteItem.h"
#include "core/math/point.h"
#include "core/math/rect.h"

namespace sc {
    namespace Adobe {
        class SlicedItem : public BitmapItem {
        public:
            SlicedItem(Animate::Publisher::SymbolContext& context,
                       wk::RawImageRef image,
                       const Animate::DOM::Utils::MATRIX2D& matrix,
                       const wk::PointF& translation,
                       const Animate::DOM::Utils::RECT& guides);

        public:
            const wk::PointF& Translation() const { return m_translation; }

            const wk::RectF& Guides() const { return m_guides; }

            virtual bool Is9Sliced() const { return true; }

            static wk::RectF RoundScalingGrid(const Animate::DOM::Utils::RECT&);

        private:
            wk::PointF m_translation;
            wk::RectF m_guides;
        };
    }
}