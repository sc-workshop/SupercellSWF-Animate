
import Locale from "../../../Localization";
import { Settings, SWFType } from "../../../PublisherSettings";

import SubMenu from "../../Shared/SubMenu";
import TextureSettings from "./textures";
import BoolField from "../../Shared/BoolField";
import FileField from "../../Shared/FileField";
import { ReactNode, useState } from "react";
import OtherSettings from "./others";
import { GetPublishContext } from "../../../Context";

export default function SettingsMenu() {
    const [isExportToExternal, setExportToExternal] = useState(Settings.getParam("exportToExternal"));
    const { toggleBackwardCompatibility, fileType } = GetPublishContext();

    let is_sc1 = fileType == SWFType.SC1;

    const default_style = {
        marginLeft: "2%",
        marginBottom: "10px",
        display: "flex",
        alignItems: "center"
    };

    const exportToExternal = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL"),
            keyName: "export_to_external_select",
            defaultValue: Settings.getParam("exportToExternal"),
            style: default_style,
            callback: [isExportToExternal, setExportToExternal],
            tip_tid: "TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL_TIP"
        }
    ).render()
    Settings.data["exportToExternal"] = isExportToExternal;

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

    const externalFilePath = FileField(
        Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL_PATH"),
        "export_to_external_path",
        "read",
        "sc",
        default_style,
        function (value) { Settings.setParam("exportToExternalPath", value) },
        Settings.getParam("exportToExternalPath")
    );

    const repackAtlas = new BoolField(
        {
            name: Locale.Get("TID_SWF_REPACK_ATLAS"),
            keyName: "repack_atlas_select",
            defaultValue: Settings.getParam("repackAtlas"),
            style: default_style,
            callback: value => (Settings.setParam("repackAtlas", value)),
            tip_tid: "TID_SWF_REPACK_ATLAS_TIP"
        }
    )

    const lowPrecisionMatrices = new BoolField(
        {
            name: Locale.Get("TID_SWF2_LOW_PRECISION_MATRICES"),
            keyName: "low_precision_matrices_select",
            defaultValue: Settings.getParam("lowPrecisionMatrices"),
            style: default_style,
            callback: value => (Settings.setParam("lowPrecisionMatrices", value)),
            tip_tid: "TID_SWF_LOW_PRECISION_MATRICES_TIP"
        }
    )

    const shortFrames = new BoolField(
        {
            name: Locale.Get("TID_SWF2_SHORT_FRAMES"),
            keyName: "low_precision_matrices_select",
            defaultValue: Settings.getParam("useShortFrames"),
            style: default_style,
            callback: value => (Settings.setParam("useShortFrames", value))
        }
    )

    const externalFileSettings: ReactNode[] = 
        isExportToExternal ? [externalFilePath, repackAtlas.render()] 
        : [];
        
    const sc1Settings: ReactNode[] =
        is_sc1 ? [backwardCompatibility.render()] 
        : [];

    const sc2Settings: ReactNode[] = 
        !is_sc1 ? [lowPrecisionMatrices.render(), shortFrames.render()] 
        : [];

    const components = [
        exportToExternal, 
        ...externalFileSettings, 
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