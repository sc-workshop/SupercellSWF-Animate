import { getInterface, isCEP } from "CEP"; // адаптируй под свой проект
import type { BaseProps } from "Components/Shared";
import TextField from "Components/Shared/TextField";
import { createElement, useState } from "react";

type FileFieldProps = BaseProps & {
	mode: "read" | "write";
	ext: string;
	callback?: (value: string) => void;
	defaultValue?: string;
};

export default function FileField({
	name,
	keyName,
	mode,
	ext,
	style,
	callback,
	defaultValue = "",
}: FileFieldProps) {
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
			if (callback) callback(value);
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
			marginRight: "10px",
		},
		onClick: async () => {
			if (!isCEP()) {
				alert("File browsing is only available in CEP environment.");
				return;
			}

			const CSInterface = getInterface();
			const targetMode = mode == "read" ? "open" : "save";
			const pathURI = await CSInterface.evalScript(
				`fl.browseForFileURL('${targetMode}','Publish to ${ext}', 'SWF','${ext}');`,
			);

			if (typeof pathURI === "string" && pathURI !== "null") {
				const path = pathURI
					.replace("file:///", "")
					.replace(/(^[a-z])\|/i, "$1:")
					.replace(/(^[a-z ]{2,}):\/?/i, "$1/")
					.replace(/\\/g, "/")
					.replace(/%20/g, " ");

				setInput(path);

				if (callback) callback(path);
			}
		},
	});

	return createElement(
		"div",
		{
			key: `filefield_${keyName}`,
			style: style,
		},
		name !== undefined ? label : undefined,
		path,
		button,
	);
}
