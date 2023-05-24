import { createElement } from "react";
import { Stylefield } from "../Shared/Stylefield";
import { Locale } from "../../Localization";

export function Header() {
    return createElement("p",
        {
            style: {
                textAlign: "center"
            }
        },
        Stylefield(Locale.Get("TID_HEADER"),
            {
                color: "white",
                fontSize: "25px",
                fontWeight: "bolder",
                textShadow: "1px 1px black",
                textAlign: "center"
            }
        ),
    )
}