import TextField from "Components/Shared/TextField";
import Locale from "Publisher/Localization";
import { createElement, useState } from "react";

// eslint-disable-next-line @typescript-eslint/no-var-requires
const Version = require("../../../package.json").version;

export function Header() {
	const [hovered, setHovered] = useState(false);

	const label = TextField("SC", {
		color: hovered ? "#ffffff" : "#e0e0e0",
		fontSize: "32px",
		fontWeight: "bold",
		textShadow: hovered ? "0 0 6px #ffffff40" : "none",
		transition: "color 0.25s, text-shadow 0.25s",
		cursor: "default",
	});

	const versionLabel = TextField(`${Locale.Get("TID_VERSION")} ${Version}`, {
		color: hovered ? "#ffffff" : "#bfbfbf",
		fontSize: "18px",
		transition: "color 0.25s, text-shadow 0.25s",
		textShadow: hovered ? "0 0 6px #ffffff40" : "none",
		cursor: "default",
	});

	const delim = createElement("div", {
		key: "header_delim",
		style: {
			width: "2px",
			height: "36px",
			backgroundColor: "#3a3a3a",
			opacity: 0.7,
			margin: "0 20px",
		},
	});

	return createElement(
		"div",
		{
			key: "publisher_info",
			style: {
				display: "flex",
				alignItems: "center",
				justifyContent: "space-between",
				padding: "0 25px",
				height: "60px",
				backgroundColor: "#262626",
				borderBottom: "1px solid #4a4a4a",
				boxShadow: "inset 0 -1px 0 #1f1f1f",
				borderRadius: "10px",
				position: "relative",
				animation: "fadeIn 0.4s ease-out",
			},
			onMouseEnter: () => setHovered(true),
			onMouseLeave: () => setHovered(false),
		},
		createElement(
			"div",
			{ style: { display: "flex", alignItems: "center" } },
			label,
			delim,
		),
		versionLabel,
	);
}
