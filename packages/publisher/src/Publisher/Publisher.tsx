import { getInterface, isCEP } from "CEP";
import { publish } from "Components";
import BasicSettings from "Components/Publisher/BasicSettings";
import { Header } from "Components/Publisher/Header";
import SettingsMenu from "Components/Publisher/Settings";
import EnumField from "Components/Shared/EnumField";
import Button from "Components/Shared/TextButton";
import {
	ApplySettings,
	GetPublishContext,
	ReadSettings,
} from "Publisher/Context";
import Locale, { Locales } from "Publisher/Localization";
import { Settings } from "Publisher/PublisherSettings";
import { createElement, useEffect, useState } from "react";
import { loadFont } from "..";

function Publisher() {
	const context = GetPublishContext();
	const settings = ReadSettings();
	const [hovered, setHovered] = useState(false);

	useEffect(() => {
		ApplySettings(context, settings);
	}, [context, settings]);

	const delim = (
		<hr
			key="header_delim"
			style={{
				width: "99%",
				border: "2px solid #484848",
			}}
		/>
	);

	const buttonStyle = (isHovered: boolean): React.CSSProperties => ({
		transition: "all 0.25s ease",
		opacity: isHovered ? 1 : 0.65,
		filter: isHovered ? "brightness(1)" : "brightness(0.85)",
		transform: isHovered ? "scale(1.05)" : "scale(1.0)",
	});

	const available_languages = ["en_US", "ru_RU", "pl_PL"];

	const language = new EnumField({
		name: "Language",
		keyName: "language_debug_sect",
		enumeration: available_languages,
		defaultValue: available_languages[0],
		style: { margin: "0 auto", ...buttonStyle(hovered) },
		callback: (value) => {
			const intValue = parseInt(value, 10);
			const localeName = available_languages[intValue];
			const localeCode = Object.entries(Locales).find(
				([key, _]) => key === localeName,
			)?.[1];
			if (localeCode !== undefined) {
				Locale.code = localeCode;
				Locale.Load();
				loadFont(() => {});
			}
		},
	});

	const publishButton = Button({
		text: Locale.Get("TID_PUBLISH"),
		keyName: "publish_start",
		style: { margin: "10px", ...buttonStyle(hovered) },
		callback: publish,
	});

	const saveSettingsButton = Button({
		text: Locale.Get("TID_SAVE_SETTINGS"),
		keyName: "save_settings_button",
		style: { margin: "10px", ...buttonStyle(hovered) },
		callback: () => {
			if (!isCEP()) return;
			Settings.save();
			getInterface().closeExtension();
		},
	});

	const inspectSettings = Button({
		text: "Inspect settings",
		keyName: "inspect_settings_button",
		style: { margin: "10px", ...buttonStyle(hovered) },
		callback: () => {
			console.log(Settings.data);
		},
	});

	const debug = process.env.NODE_ENV !== "production";
	const debugComponents = debug ? [language.render(), inspectSettings] : [];

	const buttonContainer = createElement(
		"div",
		{
			key: "button_container",
			onMouseEnter: () => setHovered(true),
			onMouseLeave: () => setHovered(false),
			style: {
				background: "#262626",
				width: "100%",
				height: hovered ? "85px" : "60px",
				display: "flex",
				position: "fixed",
				bottom: "0",
				left: "0",
				alignItems: "center",
				justifyContent: "center",
				gap: "15px",
				boxShadow: hovered
					? "0px -3px 6px #00000033"
					: "0px -2px 5px #0000001f",
				backdropFilter: "blur(8px)",
				opacity: hovered ? 1 : 0.75,
				transform: hovered ? "translateY(0px)" : "translateY(8px)",
				transition: "all 0.3s cubic-bezier(0.4, 0.0, 0.2, 1), opacity 0.3s",
				borderTop: "1px solid #333",
				zIndex: 100,
			},
		},
		publishButton,
		saveSettingsButton,
		...debugComponents,
	);

	const childrens = [
		Header(),
		delim,
		BasicSettings(),
		SettingsMenu(),
		buttonContainer,
	];

	return createElement(
		"div",
		{
			id: "publisher",
			style: {
				width: "100%",
				height: "100%",
				userSelect: "none",
				MozUserSelect: "none",
				KhtmlUserSelect: "none",
				WebkitUserSelect: "none",
				overflow: "hidden",
			},
		},
		...childrens,
	);
}

export default Publisher;
