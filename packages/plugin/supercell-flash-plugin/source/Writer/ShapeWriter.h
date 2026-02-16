#pragma once

#include "AnimatePublisher.h"
#include "core/image/raw_image.h"
#include "core/math/point.h"

#include "VectorRasterizer.h"

namespace sc::Adobe {
    class SCWriter;

    class SCShapeWriter : public Animate::Publisher::SharedShapeWriter {
    public:
        SCShapeWriter(SCWriter& writer, Animate::Publisher::SymbolContext& symbol) :
            Animate::Publisher::SharedShapeWriter(symbol),
            m_writer(writer), m_rasterizer(writer) {};
        virtual ~SCShapeWriter() = default;

        /// @brief 9Slice shapes resolution value
        static inline const float RasterizationResolution = 2.f;

    public:
        virtual void AddGraphic(const Animate::Publisher::BitmapElement& item);
        virtual void AddFilledElement(const Animate::Publisher::FilledElement& shape);
        virtual void AddSlicedElements(const Animate::Publisher::Slice9Element& slice);

        virtual void PreFinalize() override;
        virtual bool Finalize(Animate::Publisher::ResourceReference reference, bool required, bool new_symbol);

    protected:
        virtual std::size_t GenerateHash() const;

    public:
        void AddContourRegion(const Animate::Publisher::FilledElementPath& contour,
                                   const Animate::DOM::Utils::MATRIX2D& matrix,
                                   const Animate::DOM::Utils::COLOR& color);

        void AddTriangulatedRegion(const Animate::Publisher::FilledElementPath& contour,
                                   const std::vector<Animate::Publisher::FilledElementPath>& holes,
                                   const Animate::DOM::Utils::MATRIX2D& matrix,
                                   const Animate::DOM::Utils::COLOR& color);

        void FlushRasterizer();

    public:
        void AddFilledShapeRegion(const Animate::Publisher::FilledElementRegion& region,
                                  const Animate::DOM::Utils::MATRIX2D& matrix);
        bool IsValidFilledShapeRegion(const Animate::Publisher::FilledElementRegion& region);
        bool IsComplexShapeRegion(const Animate::Publisher::FilledElementRegion& region);

    private:
        SCWriter& m_writer;
        Animate::Publisher::StaticElementsGroup m_group;

        // Queue of vector shapes rasterization
        VectorRasterizer m_rasterizer;

        // Current 9slice shape guides
        std::optional<VectorBound> m_guides;
    };
}
