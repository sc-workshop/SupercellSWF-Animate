import { createElement } from "react";
import { getParam, setParam } from "../publisherState";
import { Filefield } from "../Shared/Filefield";
import { theme, themes } from "../themes";
import { Locale } from "../../Localization";

export function BasicSettings() {

    return createElement(
        "div",
        {
            id: "BasicSettings",
            style: {
                backgroundColor: theme === themes.Dark ? "#272727" : "#DCDCDC",
                width: "100%",
                paddingBottom: "5%"
            }
        },
        Filefield(
            Locale.Get("TID_OUTPUT"),
            getParam("debug") === "1" ? "json" : "sc",
            value => (setParam("output", value)),
            getParam("output")
        ),
    )

}