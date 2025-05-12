import { createElement } from "react";
import { Settings, SWFType } from "PublisherSettings";
import Locale from "Localization";

import FileField from "../Shared/FileField";
import EnumField from "../Shared/EnumField";
import { GetPublishContext } from "Context";

export default function BasicSettings() {
    const { setFileType } = GetPublishContext();

    const output = FileField(
        Locale.Get("TID_OUTPUT"),
        "publisher_output_path",
        "write",
        "sc",
        {
            marginBottom: "10px",
            display: "flex",
            alignItems: " center"
        },
        value => (Settings.setParam("output", value)),
        Settings.getParam("output")
    )

    const filetype = new EnumField({
        name: Locale.Get("TID_SWF_SETTINGS_FILETYPE"),
        keyName: "swf_type",
        enumeration: SWFType,
        defaultValue: Settings.getParam("type"),
        style: {
            display: "flex",
            alignItems: "center",
            marginBottom: "10px"
        },
        callback: value => (setFileType(parseInt(value))),
    }).render()

    return createElement(
        "div",
        {
            key: "publisher_basic_settings",
            style: {
                width: "100%"
            }
        },
        output,
        filetype
    )
}
