
import Locale from "Localization";
import { Settings, SWFType } from "PublisherSettings";

import SubMenu from "Components/Shared/SubMenu";
import BoolField from "Components/Shared/BoolField";
import FileField from "Components/Shared/FileField";
import TextureSettings from "./textures";
import React, { ReactNode, useState } from "react";
import OtherSettings from "./others";
import { GetPublishContext } from "Context";
import { renderComponents } from "Publisher";

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
    )
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

    const externalFilePath = new FileField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL_PATH"),
            keyName: "export_to_external_path",
            mode: "read",
            ext: "sc",
            style: default_style,
            callback: function (value) { Settings.setParam("exportToExternalPath", value) },
            defaultValue: Settings.getParam("exportToExternalPath")
        }
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
            keyName: "short_frames_select",
            defaultValue: Settings.getParam("useShortFrames"),
            style: default_style,
            callback: value => (Settings.setParam("useShortFrames", value))
        }
    )

    const common = renderComponents(
        [exportToExternal]
    )

    const externalFileSettings = renderComponents(
        [externalFilePath, repackAtlas] , isExportToExternal
    );
        
    const sc1Settings = renderComponents(
        [backwardCompatibility],
        is_sc1
    );

    const sc2Settings = renderComponents(
        [lowPrecisionMatrices, shortFrames], 
        !is_sc1
    );

    const components = [
        ...common, 
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