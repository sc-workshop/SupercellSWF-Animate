import { createElement, useState } from "react";
import { State } from "../publisherState";
import Locale from "../../Localization";
import { PublisherMethods } from "../publisherState";

import EnumField from "../Shared/EnumField";
import FileField from "../Shared/FileField";

export default function BasicSettings() {
    const [fileExtension, setFileExtension] = useState("sc");

    const output = FileField(
        Locale.Get("TID_OUTPUT"),
        "publisher_output_path",
        fileExtension,
        {
            marginBottom: "6px"
        },
        value => (State.setParam("output", value)),
        State.getParam("output")
    )

    const method = EnumField(
        Locale.Get("TID_PUBLISH_METHOD"),
        "publisher_method",
        PublisherMethods,
        PublisherMethods[State.getParam("method")],
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
            State.setParam("method", PublisherMethods[value as any])
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
        method

    )
}
