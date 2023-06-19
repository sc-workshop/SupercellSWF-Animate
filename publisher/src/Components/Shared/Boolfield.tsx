import { CSSProperties, Component, createElement, useState } from "react";
import TextField from "./TextField";

let id = 0;
export default function BoolField(
    name: string,
    defaultValue: boolean,
    style: CSSProperties,
    onChange: (value: boolean
    ) => void) {
    const [isFocus, setIsFocus] = useState(false);
    const fieldId = id++;

    const label = TextField(
        `${name} :`,
        {
            color: "#727776",
            marginRight: "5px"
        }
    );

    const checkbox = createElement(
        "input",
        {
            key: `boolfield_${fieldId}_input`,
            type: "checkbox",
            onChange: function (event: React.FormEvent<HTMLInputElement>) {
                onChange(event.currentTarget.checked);
            },
            style: {
                width: `15px`,
                height: "15px",
                accentColor: "black",
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
    );

    return createElement(
        "div",
        {
            key: `boolfield_${fieldId}`,
            style: style
        },
        label,
        checkbox
    )
}