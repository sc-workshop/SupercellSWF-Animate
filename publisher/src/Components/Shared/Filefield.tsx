import { createElement, useState } from "react";

import { Stylefield } from "./Stylefield";
import { getInterface, isCEP } from "../../CEP";

export function Filefield(name: string, ext: string, callback: (value: any) => void, defaultValue: string = "") {
    const [input, setInput] = useState("");

    const outputLabel = Stylefield(
        `${name} :`,
        {
            width: "70px",
            textAlign: "right",
            display: "block",
            float: "left",
            paddingTop: "4px",
            paddingRight: "11px"
        }
    );

    const filepath = createElement("input", {
        key: `Filefield_${name}_filepath`,
        type: "text",
        defaultValue: defaultValue,
        value: input,
        onChange: (e: React.ChangeEvent<HTMLInputElement>) => {
            callback(e.target.value);
        },
        style: {
            background: "#c6c6c6",
            border: "1px solid #070707",
            float: "left",
            display: "block",
            width: "350px",
            color: "#444444",
            borderColor: "solid #D49B00"
        }
    });

    const openFileButton = createElement("input", {
        key: `Filefield_${name}_openFileButton`,
        type: "image",
        src: "folderOpen.png",
        style: {
            width: "16px",
            height: "16px",
            borderWidth: "1px",
            borderColor: "solid #070707",
            marginLeft: "5px",
            marginTop: "3px",
            left: "10px"
        },
        onClick: function () {
            if (!isCEP()) {
                return;
            }
            const CSInterface = getInterface();

            CSInterface.evalScript(`fl.browseForFileURL('save','Publish to ${ext}', 'SWF','${ext}');`,
                function (path: string) {
                    setInput(path);
                }
            );
        }
    });

    return createElement(
        "p",
        { key: `Filefield_${name}` },
        outputLabel, filepath, openFileButton,
        <br />);
}