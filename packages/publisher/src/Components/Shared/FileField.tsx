import { getInterface, isCEP } from "CEP";
import { type CSSProperties, createElement, useState } from "react";
import TextField from "./TextField";

type FileFieldMode = "read" | "write";

export default function FileField(
	name: string,
	keyName: string,
	mode: FileFieldMode,
	ext: string,
	style: CSSProperties,
	callback: (value: string) => void,
	defaultValue: string = "",
) {
	const [input, setInput] = useState(defaultValue);
	const [isFocus, setIsFocus] = useState(false);

	const label = TextField(`${name} :`, {
		color: "#727776",
	});

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
			marginLeft: "5px",
		},
		onChange: (event) => {
			const { value } = event.target;
			setInput(value);
			callback(value);
		},
		onFocus: () => {
			setIsFocus(true);
		},
		onBlur: () => {
			setIsFocus(false);
		},
	});

	const button = createElement("input", {
		key: `filefield_${keyName}_button`,
		type: "image",
		src: require(`../../Assets/images/folderOpen.png`),
		style: {
			width: "20px",
			height: "20px",
			marginLeft: "5px",
		},
		onClick: async () => {
			if (!isCEP()) {
				return;
			}
			const CSInterface = getInterface();

			const pathURI = await CSInterface.evalScript(
				`fl.browseForFileURL('${mode == "read" ? "open" : "save"}','Publish to ${ext}', 'SWF','${ext}');`,
			);
			if (typeof pathURI === "string" && pathURI !== "null") {
				const path = pathURI
					.replace("file:///", "")
					.replace(/(^[a-z])\|/i, "$1:")
					.replace(/(^[a-z ]{2,}):\/?/i, "$1/")
					.replace(/\\/g, "/")
					.replace(/%20/g, " ");

				setInput(path);
				callback(path);
			}
		},
	});

	return createElement(
		"div",
		{
			key: `filefield_${keyName}`,
			style: style,
		},
		label,
		path,
		button,
	);
}
