import { createElement, useState } from "react";
import { Settings } from "../../PublisherSettings";
import Locale from "../../Localization";

import EnumField from "../Shared/EnumField";
import FileField from "../Shared/FileField";

export default function BasicSettings() {
    const [fileExtension, setFileExtension] = useState("sc");

    const output = FileField(
        Locale.Get("TID_OUTPUT"),
        "publisher_output_path",
        "write",
        fileExtension,
        {
            marginBottom: "7px"
        },
        value => (Settings.setParam("output", value)),
        Settings.getParam("output")
    )

    return createElement(
        "div",
        {
            key: "publisher_basic_settings",
            style: {
                width: "100%"
            }
        },
        output

    )
}
