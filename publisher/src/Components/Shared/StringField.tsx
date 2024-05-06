import { CSSProperties, createElement, useState } from "react";

import TextField from "./TextField";

export default function StringField(
    name: string,
    keyName: string,
    style: CSSProperties,
    callback: (value: string) => void,
    defaultValue: string = "") {
    const [input, setInput] = useState(defaultValue);
    const [isFocus, setIsFocus] = useState(false);

    const label = TextField(
        `${name} :`,
        {
            color: "#727776",
        }
    );

    const path = createElement("input", {
        key: `stringfield_${keyName}_input`,
        type: "text",
        value: input,
        style: {
            width: "285px",
            height: "30px",
            background: "#c6c6c6",
            color: "white",
            border: isFocus ? "4px solid #337ed4" : "4px solid #363636",
            borderRadius: "5px",
            outline: "none",
            backgroundColor: "black",
            marginLeft: "5px"
        },
        onChange: function (event) {
            const {value} = event.target;
            setInput(value);
            callback(value);
        },
        onFocus: function () {
            setIsFocus(true);
        },
        onBlur: function () {
            setIsFocus(false);
        }
    });
    return createElement(
        "div",
        {
            key: `stringfield_${keyName}`,
            style: style
        },
        label,
        path,

    );
}