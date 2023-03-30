import { createElement } from "react";
import { CSInterface } from "../CEP";
import { Stylefield } from "./Stylefield";

export function Filefield(name: string, ext: string, callback: (value: any) => void, defaultValue: string = "") {
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
            if (!CSInterface) {
                return;
            }
            CSInterface.evalScript(`fl.browseForFileURL('save','Publish to ${ext}', 'SWF','${ext}');`,
                function (path: string) {
                    filepath.props.value = path;
                });
        }
    });

    return createElement(
        "p",
        { key: `Filefield_${name}` },
        outputLabel, filepath, openFileButton,
        <br />);
}