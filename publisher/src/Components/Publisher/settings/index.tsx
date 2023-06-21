
import Locale from "../../../Localization";
import { State } from "../../publisherState";
import { CompressionMethods } from "../../publisherState"

import SubMenu from "../../Shared/SubMenu";
import EnumField from "../../Shared/EnumField";
import TextureSettings from "./textures";
import BoolField from "../../Shared/BoolField";
import FileField from "../../Shared/FileField";
import { useState } from "react";

export default function Settings() {
    const [isExportToExternal, setExportToExternal] = useState(State.getParam("exportToExternal"));

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

    let childrens = [
        compressionType,
        exportToExternal,
        TextureSettings()
    ]

    let externalFilePath = FileField(
        Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL_PATH"),
        "export_to_external_path",
        "sc",
        {
            marginLeft: "2%",
            marginBottom: "6px"
        },
        function (value) { State.setParam("exportToExternalPath", value) },
        State.getParam("exportToExternalPath")
    )


    if (isExportToExternal) {
        childrens.splice(2, 0, externalFilePath);
    }

    return SubMenu(
        Locale.Get("TID_ADDITIONAL_SETTINGS_LABEL"),
        "additional_settings",
        {
            marginBottom: "12px"
        },
        childrens
    )
}