import { createElement } from "react";
import { Locale } from "../../Localization";
import { Stylefield } from "../Shared/Stylefield";
import { getParam, setParam } from "../publisherState";
import { Enumfield } from "../Shared/Enumfield";

enum CompressionMethods {
    LZMA,
    LZHAM,
    ZSTD
}

export function SWFSettings() {
    const hasTexture = createElement(
        "p",
        {

        },
        Stylefield(Locale.Get("TID_SWF_SETTINGS_HAS_TEXTURE"),
            {
                color: "white",
                fontSize: "15px",

                //textAlign: "left"
            }
        ),
        createElement(
            "input",
            {
                type: "checkbox",
                onChange: function (event) {
                    setParam(
                        "hasTexture",
                        event.currentTarget.checked ? "1" : "0"
                    );
                },
            }
        )
    )

    const compressionSelector = createElement(
        "p",
        {

        },
        Stylefield(Locale.Get("TID_SWF_SETTINGS_COMPRESSION"),
            {
                color: "white",
                fontSize: "15px",

                //textAlign: "left"
            }
        ),
        Enumfield(
            CompressionMethods,
            getParam("compression"),
            {

            },
            function (value) {
                setParam("compression", value);
            }
        )
    )

    return createElement(
        "div",
        {

        },
        hasTexture,
        compressionSelector
    )

}