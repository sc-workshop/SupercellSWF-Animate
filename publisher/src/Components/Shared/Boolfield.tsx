import { CSSProperties, createElement, useState } from "react";
import { Stylefield } from "./Stylefield";

export function Boolfield(
    name: string,
    defaultValue: boolean,
    style: CSSProperties,
    onChange: (value: boolean) => void
    ) {
    const [isFocus, setIsFocus] = useState(false);

    const label = Stylefield(
        `${name} :`,
        {
            textAlign: "match-parent",
            display: "block",
            float: "left",
            color: "#727776",
            paddingTop: "4px",
            paddingRight: "11px",
            margin: "auto",
        }
    );

    return createElement(
        "div",
        {
            key: "Enumfield",
            style: style
        },
        label,
        createElement(
            "input",
            {
                type: "checkbox",
                onChange: function (event: React.FormEvent<HTMLInputElement>) {
                    onChange(event.currentTarget.checked);
                },
                style: {
                    width: `15px`,
                    height: "15px",
                    accentColor: "black",
                    marginTop: "7px",
                    color: "black",
                    backgroundColor: "black",
                    borderRadius: "30px",
                    outline: isFocus ? "2px solid #337ed4" : "none",
                    border: "2px solid #363636",
                },
                defaultValue: defaultValue,
                defaultChecked: defaultValue,
                onFocus: function () {
                    setIsFocus(true);
                },
                onBlur: function () {
                    setIsFocus(false);
                }
            }
        )
    )
}