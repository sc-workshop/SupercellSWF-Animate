import { CSSProperties, ReactNode, createElement } from "react";
import { AppTheme, AppThemes } from "../themes";
import { useState } from "react";
import { Stylefield } from "./Stylefield";

export function DropdowMenu(name: string, style: CSSProperties = {}, ...items: ReactNode[]) {
    const [active, setActive] = useState(false);

    const itemStorage = createElement(
        "div",
         {
            style: {
                paddingLeft: "5%"
            }
         },
          ...items);

    const openButton = createElement("input", {
        type: "image",
        src: `${AppThemes[AppTheme]}/arrow.png`,
        style: {
            width: "15px",
            height: "10px",
            scale: active ? "1 -1" : "1 1",
            verticalAlign: "middle"
        },
        onClick: function (event: React.MouseEvent<HTMLElement>) {
            setActive(active === true ? false : true);
        }
    });

    const label = Stylefield(
        name,
        {
            color: "white",
            paddingLeft: "10px",
            fontSize: "18px"
        }
    );

    if (active) {
        return createElement(
            "div",
            {
                style: style
            },
            openButton,
            label,
            itemStorage
        )
    } else {
        return createElement(
            "div",
            {
                style: style
            },
            openButton,
            label
        )
    }

    
}