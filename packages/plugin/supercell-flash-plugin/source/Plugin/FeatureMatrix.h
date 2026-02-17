#pragma once

#include "AnimateModule.h"

namespace sc::Adobe {
    using FeatureMatrix = Animate::DocType::GenericFeatureMatrix;

    class SCFeatureMatrixLoader : public Animate::DocType::GenericFeatureMatrixLoader<FeatureMatrix> {
    public:
        virtual void LoadMatrix(FeatureMatrix* matrix);
    };
}