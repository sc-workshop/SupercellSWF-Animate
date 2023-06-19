import { CSSProperties, createElement, useState } from "react";

import TextField from "./TextField";
import { getInterface, isCEP } from "../../CEP";
import { AppTheme, AppThemes } from "../themes";

export default function FileField(
    name: string,
    keyName: string,
    ext: string,
    style: CSSProperties,
    callback: (value: any) => void,
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
        key: `filefield_${keyName}_input`,
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
            const value = event.target.value;
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

    const button = createElement(
        "input",
        {
            key: `filefield_${keyName}_button`,
            type: "image",
            src: require(`../../images/${AppThemes[AppTheme]}/folderOpen.png`),
            style: {
                width: "20px",
                height: "20px",
                position: 'relative',
                top: "3px",
                left: "4px"
            },
            onClick: async function (event: React.MouseEvent<HTMLElement>) {
                if (!isCEP()) {
                    return;
                }
                const CSInterface = getInterface();

                const selectFileEvent = new Promise((resolve, reject) => {
                    CSInterface.evalScript(`fl.browseForFileURL('save','Publish to ${ext}', 'SWF','${ext}');`,
                        function (path: string) {
                            const outputPath = path
                                .replace('file:///', '')
                                .replace(/(^[a-z])\|/i, '$1:')
                                .replace(/(^[a-z ]{2,}):\/?/i, '$1/')
                                .replace(/\\/g, '/')
                                .replace(/%20/g, ' ');

                            resolve(outputPath);
                        }
                    );
                })

                const outputPath = await selectFileEvent;
                if (typeof outputPath === "string" && outputPath !== "null") {
                    setInput(outputPath)
                    callback(outputPath);
                }
            }
        });

    return createElement(
        "div",
        {
            key: `filefield_${keyName}`,
            style: style
        },
        label,
        path,
        button

    );
}