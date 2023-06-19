import { CSSProperties, createElement, useState } from "react";
import TextField from "./TextField";

interface Enum {
    [id: number]: any;
    [id: string]: any;
}

type EnumArray = string[] | number[];

export default function EnumField(
    name: string,
    keyName: string,
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
                <option key={`enumfield_${keyName}_${key}`} value={i}>{key}</option>
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
                <option key={`enumfield_${keyName}_${enumValue}`} value={enumValue}>{enumKey}</option>
            );
        }
    }

    const label = TextField(
        `${name} :`,
        {
            color: "#727776",
        }
    )

    const selector = createElement(
        "select",
        {
            key: `enumfield_${keyName}_input`,
            style: {
                width: `${maxEnumSize * 15}px`,
                height: "35px",
                color: "white",
                marginLeft: "5px",
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

    return createElement(
        "div",
        {
            key: `enumfield_${keyName}`,
            style: style
        },
        label,
        selector
    )
}