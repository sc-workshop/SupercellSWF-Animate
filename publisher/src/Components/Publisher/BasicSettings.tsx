import { createElement, useState } from "react";
import { Settings } from "../../PublisherSettings";
import Locale from "../../Localization";
import { PublisherMethods } from "../../PublisherSettings";

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
            marginBottom: "6px"
        },
        value => (Settings.setParam("output", value)),
        Settings.getParam("output")
    )

    const method = EnumField(
        Locale.Get("TID_PUBLISH_METHOD"),
        "publisher_method",
        PublisherMethods,
        Settings.getParam("method"),
        {
            marginBottom: "6px"
        },
        function (value) {
            const key: PublisherMethods = parseInt(value);
            switch (key) {
                case PublisherMethods.SWF:
                    setFileExtension("sc")
                    break
                case PublisherMethods.JSON:
                    setFileExtension("json")
                    break
            }
            Settings.setParam("method", key)
        }
    );

    return createElement(
        "div",
        {
            key: "publisher_basic_settings",
            style: {
                width: "100%"
            }
        },
        output,
        process.env.NODE_ENV == "production" ? undefined : method

    )
}
