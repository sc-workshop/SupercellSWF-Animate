import { createElement, useState } from "react";

import { Stylefield } from "./Stylefield";
import { getInterface, isCEP } from "../../CEP";

export function Filefield(name: string, ext: string, callback: (value: any) => void, defaultValue: string = "") {
    const [input, setInput] = useState(defaultValue);
    const [isFocus, setIsFocus] = useState(false);

    const outputLabel = Stylefield(
        `${name} :`,
        {
            color: "#727776",
            paddingTop: "4px",
            paddingRight: "11px",
            margin: "auto"
        }
    );

    const filepath = createElement("input", {
        key: `Filefield_${name}_filepath`,
        type: "text",
        value: input,
        style: {
            background: "#c6c6c6",
            width: "275px",
            color: "white",
            border: isFocus ? "4px solid #337ed4" : "3px solid #363636",
            borderRadius: "5px",
            outline: "none",
            height: "30px",
            backgroundColor: "black",
            margin: isFocus ? "-4px" : "-3px"
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

    const openFileButton = createElement("input", {
        key: `Filefield_${name}_openFileButton`,
        type: "image",
        src: "folderOpen.png",
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

            const outputPath = await selectFileEvent as string;
            setInput(outputPath)
            callback(outputPath);
        }
    });

    return createElement(
        "div",
        {
            key: `Filefield_${name}`
        },
        outputLabel,
        filepath,
        openFileButton

    );
}