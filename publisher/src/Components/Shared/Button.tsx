import { CSSProperties, createElement, useState } from "react";
import TextField from "./TextField";

export default function Button(
    text: string,
    keyName: string,
    style: CSSProperties,
    callback: () => void
    )  {
    const [isFocus, setIsFocus] = useState(false);

    const label = TextField(
        text,
        {
            color: "#ffffff",
            marginRight: "5px",
            userSelect: "none"
        }
    );

    const button = createElement(
        "button",
        {
            key: `button_${keyName}`,
            onMouseDown: (event) => {
                event.persist();
                callback();
            },
            style: {
                width: "fit-content",
                height: "35px",
                border: "3px solid white",
                background: "rgba(0,0,0,0.0)",
                borderRadius: "20px",
                outline: isFocus ? "2px solid #337ed4" : "none",
                diplsay: "flex",
                alignItem: "center"
            },
            onFocus: function () {
                setIsFocus(true);
            },
            onBlur: function () {
                setIsFocus(false);
            }
        },
        label
    );

    return createElement("div", {style: style}, button);
}