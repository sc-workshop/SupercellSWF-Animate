#include "FeatureMatrix.h"

#include "Module/Module.h"

namespace sc::Adobe {
    void SCFeatureMatrixLoader::LoadMatrix(FeatureMatrix* matrix) {
        SCPlugin& context = SCPlugin::Instance();
        fs::path featuresPath = context.CurrentPath(SCPlugin::PathType::Assets) / "features.json";
        matrix->FromJSON(featuresPath);
    }
}