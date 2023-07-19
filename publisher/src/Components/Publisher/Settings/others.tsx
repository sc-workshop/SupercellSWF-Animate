import Locale from "../../../Localization";
import BoolField from "../../Shared/BoolField";
import EnumField from "../../Shared/EnumField";
import SubMenu from "../../Shared/SubMenu";
import { CompressionMethods, State } from "../../publisherState";

export default function OtherSettings() {
    const compressionType = EnumField(
        Locale.Get("TID_SWF_SETTINGS_COMPRESSION"),
        "file_compression_select",
        CompressionMethods,
        State.getParam("compressionMethod"),
        {
            marginBottom: "6px"
        },
        value => (State.setParam("compressionMethod", parseInt(value))),
    )
    
    const shapeOptimization = BoolField(
        Locale.Get("TID_SWF_SETTINGS_FILLED_SHAPE_OPTIMIZATION"),
        "filled_shape_optimization",
        State.getParam("filledShapeOptimization"),
        {
            marginBottom: "6px"
        },
        value => (State.setParam("filledShapeOptimization", value)),
    );

    const precisionMatrix = BoolField(
        Locale.Get("TID_SWF_SETTINGS_PRECISION_MATRIX"),
        "precision_matrix",
        State.getParam("hasPrecisionMatrices"),
        {
            marginBottom: "6px"
        },
        value => (State.setParam("hasPrecisionMatrices", value)),
    );
    
    return SubMenu(
        Locale.Get("TID_OTHER_LABEL"),
        "other_settings",
        {
            marginBottom: "20%"
        },
        compressionType,
        shapeOptimization,
        precisionMatrix,
    )
}