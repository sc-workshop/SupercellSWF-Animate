import { createElement } from "react";
import  { getParam, PublisherModes, setParam, State } from "../publisherState";
import { Filefield } from "../Shared/Filefield";
import { theme, themes } from "../themes";
import { Enumfield } from "../Shared/Enumfield";
import { Stylefield } from "../Shared/Stylefield";

export function BasicSettings() {

    return createElement("div", {
        id: "BasicSettings",
        style: {
            backgroundColor: theme === themes.Dark ? "#272727" : "#DCDCDC",
            width: "100%",
            paddingBottom: "5%"
        }
    },
        Filefield(
            "Output",
            State.PublishSettings.SupercellSWF.publisher.toLowerCase(),
            value => (setParam("output", value)),
            getParam("output")
        ),
        
        Stylefield(
            `Publish method :`,
            {
                width: "110px",
                textAlign: "right",
                display: "block",
                float: "left",
                color: "white",
                paddingRight: "2%",
                paddingLeft: "50%"
            }
        ),
        Enumfield(PublisherModes, getParam("publisher"), {
            width: "15%",
            background: "#c6c6c6",
            border: "1px solid #070707",
            display: "block",
            color: "#444444",
            borderColor: "solid #D49B00"
        },
        function(value: string) {
            setParam("publisher", value);
        })
    )

}