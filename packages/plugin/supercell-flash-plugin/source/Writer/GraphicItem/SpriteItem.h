#pragma once

#include "AnimateDOM.h"
#include "AnimatePublisher.h"
#include "GraphicItem.h"
#include "core/image/raw_image.h"
#include "core/memory/ref.h"

#include <filesystem>

namespace sc::Adobe {
    class BitmapItem : public GraphicItem {
    public:
        BitmapItem(Animate::Publisher::SymbolContext& context,
                   wk::RawImageRef image,
                   const Animate::DOM::Utils::MATRIX2D& matrix,
                   bool rasterized = false);

        BitmapItem(Animate::Publisher::SymbolContext& context);

        virtual ~BitmapItem() = default;

    public:
        virtual const wk::RawImage& Image() const { return *m_image; }

        virtual bool IsSprite() const { return true; }

        bool IsRasterizedVector() const { return m_is_rasterized; }

    protected:
        wk::RawImageRef m_image;
        bool m_is_rasterized = false;
    };
}