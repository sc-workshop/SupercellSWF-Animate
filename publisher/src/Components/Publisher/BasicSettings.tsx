import { createElement } from "react";
import { getParam, setParam, State } from "../publisherState";
import { Filefield } from "../Shared/Filefield";
import { theme, themes } from "../themes";

export function BasicSettings() {

    return createElement("div", {
        id: "BasicSettings",
        style: {
            backgroundColor: theme === themes.Dark ? "#272727" : "#DCDCDC",
            width: "100%"
        }
    },
        Filefield(
            "Output",
            State.PublishSettings.SupercellSWF.publisher,
            value => (setParam("output", value)),
            getParam("output")
        )
    )

}