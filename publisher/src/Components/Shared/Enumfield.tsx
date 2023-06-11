import { CSSProperties, createElement, useState } from "react";
import { Stylefield } from "./Stylefield";

interface Enum {
    [id: number]: any;
    [id: string]: any;
}

type EnumArray = string[] | number[];

export function Enumfield(
    name: string,
    enumeration: Enum | EnumArray,
    defaultValue: string,
    style: CSSProperties,
    onChange: (value: string) => void
    ) {
    const options = [];
    const [isFocus, setIsFocus] = useState(false);

    let maxEnumSize = 0;
    if (Array.isArray(enumeration)) {
        for (const key of enumeration) {

            const keyStr = String(key);
            if (keyStr.length > maxEnumSize) {
                maxEnumSize = keyStr.length;
            }
            
            options.push(createElement("option", {

                value: key,
                style: {

                }
            }, key))
        }
    } else {
        for (let i = 0; Object.keys(enumeration).length / 2 > i; i++) {
            const enumKey = enumeration[i];

            const keyStr = String(enumKey);
            if (keyStr.length > maxEnumSize) {
                maxEnumSize = keyStr.length;
            }

            options.push(createElement("option", {

                value: enumKey,
                style: {

                }
            }, enumKey))
        }
    }

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
            style: style
        },
        label,
        createElement(
            "select",
            {
                style: {
                    width: `${maxEnumSize * 15}px`,
                    height: "35px",
                    color: "white",
                    marginTop: "-5px",
                    background: "#1a1a1a",
                    borderRadius: "5px",
                    outline: isFocus ? "2px solid #337ed4" : "none",
                    border: "2px solid #363636",
                },
                defaultValue: defaultValue,
                onChange: function (event: React.FormEvent<HTMLInputElement>) {
                    onChange(event.currentTarget.value)
                },
                onFocus: function () {
                    setIsFocus(true);
                },
                onBlur: function () {
                    setIsFocus(false);
                }
            },
            options
        )
    )
}