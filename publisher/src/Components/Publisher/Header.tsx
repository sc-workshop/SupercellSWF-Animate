import { createElement } from "react";
import { Stylefield } from "../Shared/Stylefield";

export function Header() {
    return createElement("p",
        {

        },
        Stylefield("SupercellSWF Publisher",
            {
                color: "white",
                fontSize: "25px",
                fontWeight: "bolder",
                textShadow: "1px 1px black"
            }
        ),
    )
}