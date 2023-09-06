
import Locale from "../../../Localization";
import { Settings } from "../../../PublisherSettings";

import SubMenu from "../../Shared/SubMenu";
import TextureSettings from "./textures";
import BoolField from "../../Shared/BoolField";
import FileField from "../../Shared/FileField";
import { useState } from "react";
import ExportsSettings from "./exports";
import OtherSettings from "./others";

export default function SettingsMenu() {
    const [isExportToExternal, setExportToExternal] = useState(Settings.getParam("exportToExternal"));

    const exportToExternal = BoolField(
        Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL"),
        "export_to_external_select",
        Settings.getParam("exportToExternal"),
        {
            marginBottom: "6px"
        },
        [isExportToExternal, setExportToExternal]
    )
    Settings.data["exportToExternal"] = isExportToExternal;

    let externalFilePath = FileField(
        Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL_PATH"),
        "export_to_external_path",
        "read",
        "sc",
        {
            marginLeft: "2%",
            marginBottom: "6px"
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
        ExportsSettings(),
        TextureSettings(),
        OtherSettings()
    )
}