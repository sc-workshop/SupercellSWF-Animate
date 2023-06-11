import { createElement, useState } from "react";
import { State } from "../publisherState";
import { Filefield } from "../Shared/Filefield";
import { Locale } from "../../Localization";
import { Enumfield } from "../Shared/Enumfield";
import { Boolfield } from "../Shared/Boolfield";

enum CompressionMethods {
    LZMA,
    LZHAM,
    ZSTD
}

export function BasicSettings() {
    const [debugMode, setDebugMode] = useState(false);

    const outputPath = Filefield(
        Locale.Get("TID_OUTPUT"),
        debugMode ? "json" : "sc",
        value => (State.setParam("output", value)),
        State.getParam("output")
    )

    const compressionType = Enumfield(
        Locale.Get("TID_SWF_SETTINGS_COMPRESSION"),
        CompressionMethods,
        State.getParam("compression"),
        {
            marginTop: "10px"
        },
        value => (State.setParam("compression", value)),
    )

    const exportToExternal = Boolfield(
        Locale.Get("TID_SWF_SETTINGS_HAS_TEXTURE"),
        State.getParam("hasTexture"),
        {
            marginTop: "10px"
        },
        value => (State.setParam("hasTexture", value)),
    )

    return createElement(
        "div",
        {
            id: "BasicSettings",
            style: {
                width: "100%",
                paddingBottom: "5%"
            }
        },
        outputPath,
        compressionType,
        exportToExternal

    )

}