import { createElement, useState } from "react";
import { Settings, SWFType } from "PublisherSettings";
import Locale from "Localization";

import FileField from "../Shared/FileField";
import EnumField from "../Shared/EnumField";
import { GetPublishContext } from "Context";
import { renderComponents } from "Publisher";
import BoolField from "../Shared/BoolField";

export default function BasicSettings() {
    const { setFileType } = GetPublishContext();
    const [isExportToExternal, setExportToExternal] = useState(Settings.getParam("exportToExternal"));

    let defaultStyle = {
            display: "flex",
            alignItems: "center",
            marginBottom: "10px"
        };

    const output = FileField(
        Locale.Get("TID_OUTPUT"),
        "publisher_output_path",
        "write",
        "sc",
        defaultStyle,
        value => (Settings.setParam("output", value)),
        Settings.getParam("output")
    )

    const filetype = new EnumField({
        name: Locale.Get("TID_SWF_SETTINGS_FILETYPE"),
        keyName: "swf_type",
        enumeration: SWFType,
        defaultValue: Settings.getParam("type"),
        style: defaultStyle,
        callback: value => (setFileType(parseInt(value))),
    })

    const exportToExternal = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL"),
            keyName: "export_to_external_select",
            defaultValue: Settings.getParam("exportToExternal"),
            style: defaultStyle,
            callback: [isExportToExternal, setExportToExternal],
            tip_tid: "TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL_TIP"
        }
    )
    Settings.data["exportToExternal"] = isExportToExternal;

    const externalFileOptionsStyle = {
            display: "flex",
            alignItems: "center",
            marginBottom: "10px",
            marginLeft: "2%"
        };

    const externalFilePath = FileField(
        Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL_PATH"),
        "export_to_external_path",
        "read",
        "sc",
        externalFileOptionsStyle,
        function (value) { Settings.setParam("exportToExternalPath", value) },
        Settings.getParam("exportToExternalPath")
    );

    const autoSettings = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_AUTO"),
            keyName: "auto_settings_select",
            defaultValue: Settings.getParam("exportToExternal"),
            style: externalFileOptionsStyle,
            callback: () => {},
            tip_tid: "TID_SWF_SETTINGS_AUTO_TIP"
        }
    )

    const repackAtlas = new BoolField(
        {
            name: Locale.Get("TID_SWF_REPACK_ATLAS"),
            keyName: "repack_atlas_select",
            defaultValue: Settings.getParam("repackAtlas"),
            style: externalFileOptionsStyle,
            callback: value => (Settings.setParam("repackAtlas", value)),
            tip_tid: "TID_SWF_REPACK_ATLAS_TIP"
        }
    )

    const externalFileSettings = renderComponents(
        [externalFilePath, autoSettings, repackAtlas], isExportToExternal
    );

    let props = renderComponents([
        output,
        filetype,
        exportToExternal,
        ...externalFileSettings
    ])

    return createElement(
        "div",
        {
            key: "publisher_basic_settings",
            style: {
                width: "100%"
            }
        },
        ...props
    )
}
