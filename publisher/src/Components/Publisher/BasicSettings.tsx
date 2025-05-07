import { createElement } from "react";
import { Settings, SWFType } from "PublisherSettings";
import Locale from "Localization";

import FileField from "../Shared/FileField";
import EnumField from "../Shared/EnumField";
import { GetPublishContext } from "Context";

export default function BasicSettings() {
    const { setFileType } = GetPublishContext();

    const output = new FileField(
        {
            name: Locale.Get("TID_OUTPUT"),
            keyName: "publisher_output_path",
            mode: "write",
            ext: "sc",
            style: {
                marginBottom: "10px",
                display: "flex",
                alignItems: " center"
            },
            callback: value => (Settings.setParam("output", value)),
            defaultValue: Settings.getParam("output"),
        }
    ).render()

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
