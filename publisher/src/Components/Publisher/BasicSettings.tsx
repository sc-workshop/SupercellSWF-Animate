import { createElement, useState } from "react";
import { State } from "../publisherState";
import { Filefield } from "../Shared/Filefield";
import { Locale } from "../../Localization";
import { Enumfield } from "../Shared/Enumfield";
import { PublisherMethods } from "../publisherState";

export function BasicSettings() {
    const [fileExtension, setFileExtension] = useState("sc");

    const outputPath = Filefield(
        Locale.Get("TID_OUTPUT"),
        fileExtension,
        value => (State.setParam("output", value)),
        State.getParam("output")
    )

    const publishMethod = Enumfield(
        Locale.Get("TID_PUBLISH_METHOD"),
        PublisherMethods,
        PublisherMethods[State.getParam("method")],
        {

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
        });

    return createElement(
        "div",
        {
            id: "BasicSettings",
            style: {
                width: "100%",
                paddingBottom: "10px"
            }
        },
        outputPath,
        publishMethod

    )

}