
import Locale from "Publisher/Localization";
import { Settings, SWFType } from "Publisher/PublisherSettings";

import SubMenu from "Components/Shared/SubMenu";
import BoolField from "Components/Shared/BoolField";
import TextureSettings from "./textures";
import OtherSettings from "./others";
import { GetPublishContext } from "Publisher/Context";
import { renderComponents } from "Publisher/Publisher";

export default function SettingsMenu() {
    const { toggleBackwardCompatibility, fileType, useAutoProperties } = GetPublishContext();

    let is_sc1 = fileType == SWFType.SC1;

    const default_style = {
        marginLeft: "2%",
        marginBottom: "10px",
        display: "flex",
        alignItems: "center"
    };

    const backwardCompatibility = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_BACKWARD_COMPATIBILITY"),
            keyName: "backward_compatibility_select",
            defaultValue: Settings.getParam("backwardCompatibility"),
            style: default_style,
            callback: toggleBackwardCompatibility,
            tip_tid: "TID_SWF_SETTINGS_BACKWARD_COMPATIBILITY_TIP"
        }
    )

    const lowPrecisionMatrices = new BoolField(
        {
            name: Locale.Get("TID_SWF2_LOW_PRECISION_MATRICES"),
            keyName: "low_precision_matrices_select",
            defaultValue: Settings.getParam("lowPrecisionMatrices"),
            style: default_style,
            callback: value => (Settings.setParam("lowPrecisionMatrices", value)),
            tip_tid: "TID_SWF2_LOW_PRECISION_MATRICES_TIP"
        }
    );
    lowPrecisionMatrices.IsAutoProperty = true;

    const shortFrames = new BoolField(
        {
            name: Locale.Get("TID_SWF2_SHORT_FRAMES"),
            keyName: "short_frames_select",
            defaultValue: Settings.getParam("useShortFrames"),
            style: default_style,
            callback: value => (Settings.setParam("useShortFrames", value))
        }
    )
    shortFrames.IsAutoProperty = true;

    const sc1Settings = renderComponents(
        [backwardCompatibility],
        is_sc1 || useAutoProperties
    );

    const sc2Settings = renderComponents(
        [lowPrecisionMatrices, shortFrames], 
        !is_sc1 || useAutoProperties
    );

    const components = [
        ...sc1Settings,
        ...sc2Settings
    ]

    return SubMenu(
        Locale.Get("TID_ADDITIONAL_SETTINGS_LABEL"),
        "additional_settings",
        {
            marginBottom: "20%"
        },
        ...components,
        TextureSettings(),
        OtherSettings()
    )
}