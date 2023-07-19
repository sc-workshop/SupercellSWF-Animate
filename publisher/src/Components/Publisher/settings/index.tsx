
import Locale from "../../../Localization";
import { State } from "../../publisherState";
import { CompressionMethods } from "../../publisherState"

import SubMenu from "../../Shared/SubMenu";
import EnumField from "../../Shared/EnumField";
import TextureSettings from "./textures";
import BoolField from "../../Shared/BoolField";
import FileField from "../../Shared/FileField";
import { useState } from "react";
import ExportsSettings from "./exports";
import OtherSettings from "./others";

export default function Settings() {
    const [isExportToExternal, setExportToExternal] = useState(State.getParam("exportToExternal"));

    const exportToExternal = BoolField(
        Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL"),
        "export_to_external_select",
        State.getParam("exportToExternal"),
        {
            marginBottom: "6px"
        },
        [isExportToExternal, setExportToExternal]
    )
    State.data["exportToExternal"] = isExportToExternal;

    let externalFilePath = FileField(
        Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL_PATH"),
        "export_to_external_path",
        "read",
        "sc",
        {
            marginLeft: "2%",
            marginBottom: "6px"
        },
        function (value) { State.setParam("exportToExternalPath", value) },
        State.getParam("exportToExternalPath")
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