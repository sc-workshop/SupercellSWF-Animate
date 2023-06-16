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
        for (let i = 0; enumeration.length > i; i++) {
            const key = String(enumeration[i]);

            if (key.length > maxEnumSize) {
                maxEnumSize = key.length;
            }

            options.push(
                <option value={i}>{key}</option>
            );
        }
    } else {
        const enumKeys = Object.keys(enumeration);
        const delimIndex = enumKeys.length / 2;

        for (let i = 0; delimIndex > i; i++) {
            const enumKey = enumKeys[i + delimIndex];
            const enumValue = enumKeys[i];

            const keyStr = String(enumKey);
            if (keyStr.length > maxEnumSize) {
                maxEnumSize = keyStr.length;
            }

            options.push(
                <option value={enumValue}>{enumKey}</option>
            );
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

    const selector = createElement(
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
    );

    return createElement(
        "div",
        {
            style: style
        },
        label,
        selector
    )
}