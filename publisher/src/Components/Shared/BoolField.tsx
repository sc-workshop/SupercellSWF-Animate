import { CSSProperties, Dispatch, SetStateAction, createElement, useState } from "react";
import TextField from "./TextField";

type callback = (value: boolean) => void;
type state = [boolean, Dispatch<SetStateAction<boolean>>];

export default function BoolField(
    name: string,
    keyName: string,
    defaultValue: boolean,
    style: CSSProperties,
    callback: callback | state
    )  {
    const [isFocus, setIsFocus] = useState(false);

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
            key: `boolfield_${keyName}_input`,
            type: "checkbox",
            onChange: function (event: React.FormEvent<HTMLInputElement>) {
                if (typeof callback == "function") {
                    callback(event.currentTarget.checked);
                } else {
                    callback[1](event.currentTarget.checked);
                }
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
            key: `boolfield_${keyName}`,
            style: style
        },
        label,
        checkbox
    )
}