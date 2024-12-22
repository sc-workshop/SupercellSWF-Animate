
import Locale from "../../../Localization";
import { Settings, SWFType } from "../../../PublisherSettings";

import SubMenu from "../../Shared/SubMenu";
import TextureSettings from "./textures";
import BoolField from "../../Shared/BoolField";
import FileField from "../../Shared/FileField";
import { useState } from "react";
import OtherSettings from "./others";
import { GetPublishContext } from "../../../Context";

export default function SettingsMenu() {
    const [isExportToExternal, setExportToExternal] = useState(Settings.getParam("exportToExternal"));
    const { toggleBackwardCompatibility, fileType } = GetPublishContext();

    let is_sc1 = fileType == SWFType.SC1;

    const exportToExternal = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL"),
            keyName: "export_to_external_select",
            defaultValue: Settings.getParam("exportToExternal"),
            style: {
                marginBottom: "10px",
                display: "flex",
                alignItems: "center"
            },
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
            style: {
                marginBottom: "10px",
                display: "flex",
                alignItems: "center"
            },
            callback: toggleBackwardCompatibility,
            tip_tid: "TID_SWF_SETTINGS_BACKWARD_COMPATIBILITY_TIP"
        }
    ).render()

    const externalFilePath = FileField(
        Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL_PATH"),
        "export_to_external_path",
        "read",
        "sc",
        {
            marginLeft: "2%",
            marginBottom: "10px",
            display: "flex",
            alignItems: "center"
        },
        function (value) { Settings.setParam("exportToExternalPath", value) },
        Settings.getParam("exportToExternalPath")
    )

    return SubMenu(
        Locale.Get("TID_ADDITIONAL_SETTINGS_LABEL"),
        "additional_settings",
        {
            marginBottom: "20%"
        },
        exportToExternal,
        isExportToExternal ? externalFilePath : undefined,
        is_sc1 ? backwardCompatibility : undefined,
        TextureSettings(),
        OtherSettings()
    )
}