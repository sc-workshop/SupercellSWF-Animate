import { CSSProperties, createElement } from "react";
import React from "react";
import TextField from "./TextField";
import { getInterface, isCEP } from "CEP";

type FileFieldMode = "read" | "write"

type Props = {
    name: string,
    keyName: string,
    mode: FileFieldMode,
    ext: string,
    style: CSSProperties,
    callback: (value: string) => void,
    defaultValue: string
}

type State = {
    focus: boolean,
    input: string
}

export default class FileField extends React.Component<Props, State> {
    state: State = {
        focus: false,
        input: ""
    };

    constructor(props: Props) {
        super(props);
    }

    render() {
        const label = TextField(
            `${this.props.name} :`,
            {
                color: "#727776",
            }
        );

        const currentElement = this;
        const path = createElement("input", {
            key: `filefield_${this.props.keyName}_input`,
            type: "text",
            value: this.state.input,
            style: {
                width: "285px",
                height: "30px",
                background: "#c6c6c6",
                color: "white",
                border: this.state.focus ? "4px solid #337ed4" : "4px solid #363636",
                borderRadius: "5px",
                outline: "none",
                backgroundColor: "black",
                marginLeft: "5px"
            },
            onChange: function (event: React.FormEvent<HTMLInputElement>) {
                const value = event.currentTarget.value;
                currentElement.state.input = value;
                currentElement.props.callback(value);
            },
            onFocus: function () {
                currentElement.state.focus = true;
            },
            onBlur: function () {
                currentElement.state.focus = false;
            }
        });

        const button = createElement(
            "input",
            {
                key: `filefield_${currentElement.props.keyName}_button`,
                type: "image",
                src: require(`../../assets/images/folderOpen.png`),
                style: {
                    width: "20px",
                    height: "20px",
                    marginLeft: "5px"
                },
                onClick: async function () {
                    if (!isCEP()) {
                        return;
                    }
                    const CSInterface = getInterface();
    
                    const pathURI = await CSInterface.evalScript(
                        `fl.browseForFileURL('${currentElement.props.mode == "read" ? "open" : "save"}','Publish to ${currentElement.props.ext}', 'SWF','${currentElement.props.ext}');`
                    );
                    if (typeof pathURI === "string" && pathURI !== "null") {
                        const path = pathURI
                                    .replace('file:///', '')
                                    .replace(/(^[a-z])\|/i, '$1:')
                                    .replace(/(^[a-z ]{2,}):\/?/i, '$1/')
                                    .replace(/\\/g, '/')
                                    .replace(/%20/g, ' ');
                        
                        currentElement.state.input = path
                        currentElement.props.callback(path);
                    }
                }
            });

        return createElement(
            "div",
            {
                key: `filefield_${this.props.keyName}`,
                style: this.props.style
            },
            label,
            path,
            button
        );
    }
}