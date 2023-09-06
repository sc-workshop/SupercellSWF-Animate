import Locale from "../../../Localization";
import BoolField from "../../Shared/BoolField";
import EnumField from "../../Shared/EnumField";
import SubMenu from "../../Shared/SubMenu";
import { CompressionMethods, Settings } from "../../../PublisherSettings";

export default function OtherSettings() {
    const compressionType = EnumField(
        Locale.Get("TID_SWF_SETTINGS_COMPRESSION"),
        "file_compression_select",
        CompressionMethods,
        Settings.getParam("compressionMethod"),
        {
            marginBottom: "6px"
        },
        value => (Settings.setParam("compressionMethod", parseInt(value))),
    )
    
    const shapeOptimization = BoolField(
        Locale.Get("TID_SWF_SETTINGS_FILLED_SHAPE_OPTIMIZATION"),
        "filled_shape_optimization",
        Settings.getParam("filledShapeOptimization"),
        {
            marginBottom: "6px"
        },
        value => (Settings.setParam("filledShapeOptimization", value)),
    );

    const precisionMatrix = BoolField(
        Locale.Get("TID_SWF_SETTINGS_PRECISION_MATRIX"),
        "precision_matrix",
        Settings.getParam("hasPrecisionMatrices"),
        {
            marginBottom: "6px"
        },
        value => (Settings.setParam("hasPrecisionMatrices", value)),
    );

    const NineSliceSprites = BoolField(
        Locale.Get("TID_SWF_SETTINGS_NINE_SLICE_SPRITES"),
        "9_slice_sprites",
        Settings.getParam("useSpritesForNineSlice"),
        {
            marginBottom: "6px"
        },
        value => (Settings.setParam("useSpritesForNineSlice", value)),
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