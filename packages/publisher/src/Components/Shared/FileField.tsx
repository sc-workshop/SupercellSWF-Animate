import { getInterface, isCEP } from "CEP";
import type { BaseProps } from "Components/Shared";
import TextField from "Components/Shared/TextField";
import { createElement, useEffect, useRef, useState } from "react";

type FileFieldProps = BaseProps & {
	mode: "read" | "write";
	ext: string;
	callback?: (value: string) => void;
	defaultValue?: string;
};

const URIToPath = (uri: string): string =>
	uri
		.replace("file:///", "")
		.replace(/(^[a-z])\|/i, "$1:")
		.replace(/(^[a-z ]{2,}):\/?/i, "$1/")
		.replace(/\\/g, "/")
		.replace(/%20/g, " ");

/**
 * Truncate file path to show meaningful information when not focused
 * @param path - Full file path to truncate
 * @param maxLength - Maximum length of displayed path
 * @returns Truncated path with contextual information
 */
const truncatePath = (path: string, maxLength: number = 35): string => {
	// If path is short enough, return as is
	if (path.length <= maxLength) return path;

	// Split path into components
	const parts = path.split(/[/\\]/);
	if (parts.length === 1) return path; // No separators found

	// Get filename and parent folder
	const fileName = parts[parts.length - 1];
	const folderName = parts.length > 1 ? parts[parts.length - 2] : "";

	// If filename is short, show parent folder context
	if (fileName.length <= 15 && folderName) {
		const combined = `${folderName}/${fileName}`;
		if (combined.length <= maxLength - 5) {
			return `.../${combined}`;
		}
	}

	// If filename is too long, truncate from the beginning
	if (fileName.length > maxLength - 5) {
		return `...${fileName.substring(fileName.length - maxLength + 5)}`;
	}

	return `...${fileName}`;
};

/**
 * Get relative path if it's shorter than absolute path
 * @param uri - Absolute path URI
 * @returns Shortest path (absolute or relative)
 */
const getShortestPath = async (uri: string) => {
	const CSInterface = getInterface();
	const path = URIToPath(uri);

	// Get current working directory
	const documentPath = await CSInterface.evalScript(
		`fl.getDocumentDOM().path;`,
	);
	if (!documentPath) return path;

	const cwd = URIToPath(documentPath).split("/").slice(0, -1).join("/");

	// Check if path is within current directory
	if (path.startsWith(cwd)) {
		// Create relative path
		const relativePath = path.substring(cwd.length).replace(/^[/\\]+/, "");
		// Return shorter path
		return relativePath.length < path.length ? relativePath : path;
	}

	return path;
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
	const [displayValue, setDisplayValue] = useState(truncatePath(defaultValue));
	const [isAnimating, setIsAnimating] = useState(false);
	const inputRef = useRef<HTMLInputElement>(null);
	const prevFocusRef = useRef(isFocus);

	/**
	 * Update display value based on focus state
	 */
	useEffect(() => {
		if (prevFocusRef.current !== isFocus) {
			// Focus state changed
			prevFocusRef.current = isFocus;
			setIsAnimating(true);

			const timer = setTimeout(() => {
				if (isFocus) {
					// Show full path when focused
					setDisplayValue(input);
				} else {
					// Show truncated path when not focused
					setDisplayValue(truncatePath(input));
				}
				setIsAnimating(false);
			}, 150);

			return () => clearTimeout(timer);
		}
	}, [isFocus, input]);

	const label = TextField(`${name} :`, {
		color: "#727776",
	});

	const path = createElement("input", {
		key: `filefield_${keyName}_input`,
		type: "text",
		value: displayValue,
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
			opacity: isAnimating ? 0.7 : 1,
			transform: isAnimating ? "translateY(2px)" : "translateY(0)",
			transition: "all 0.15s ease-in-out",
		},
		onChange: (event) => {
			const { value } = event.target;
			setInput(value);
			if (callback) callback(value);

			// When focused, update display value immediately to show what user is typing
			if (isFocus) {
				setDisplayValue(value);
			}
		},
		onFocus: () => {
			setIsFocus(true);
		},
		onBlur: () => {
			setIsFocus(false);
		},
		onMouseEnter: (e) => {
			// Show full path in tooltip when hovering over non-focused field
			if (!isFocus && input) {
				e.currentTarget.title = input;
			}
		},
		ref: inputRef,
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
			transition: "transform 0.2s ease-in-out",
		},
		onMouseEnter: (e) => {
			e.currentTarget.style.transform = "scale(1.1)";
		},
		onMouseLeave: (e) => {
			e.currentTarget.style.transform = "scale(1)";
		},
		onClick: async () => {
			if (!isCEP()) {
				alert("File browsing is only available in CEP environment.");
				return;
			}

			const CSInterface = getInterface();
			const targetMode = mode == "read" ? "open" : "save";
			const path = await CSInterface.evalScript(
				`fl.browseForFileURL('${targetMode}','Publish to ${ext}', 'SWF','${ext}');`,
			);

			if (typeof path === "string" && path !== "null") {
				// Get shortest path (absolute or relative)
				const shortestPath = await getShortestPath(path);

				setInput(shortestPath);
				setDisplayValue(shortestPath);
				if (callback) callback(shortestPath);
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
