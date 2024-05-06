import { createElement } from "react";
import { Settings } from "../../PublisherSettings";
import Locale from "../../Localization";

import FileField from "../Shared/FileField";

export default function BasicSettings() {
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
