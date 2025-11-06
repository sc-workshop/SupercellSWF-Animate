import { createElement } from "react";
import { publish } from "Components";
import { Header } from "Components/Publisher/Header";
import SettingsMenu from "Components/Publisher/Settings";
import BasicSettings from "Components/Publisher/BasicSettings";
import Button from "Components/Shared/Button";
import Locale, { Locales } from "Publisher/Localization";
import EnumField from "Components/Shared/EnumField";
import { loadFont } from "..";
import { GetPublishContext, UpdateContext } from "Publisher/Context";
import React from "react";
import DisplayObject from "Components/Shared/DisplayObject";

function Publisher() {
	UpdateContext();

	const delim = (
		<hr
			key="header_delim"
			style={{
				width: "99%",
				border: "2px solid #484848",
			}}
		/>
	);

	const publishButton = Button(
		Locale.Get("TID_PUBLISH"),
		"publish_start",
		{
			margin: "10px",
		},
		publish,
	);

	const available_languages = ["en_US", "ru_RU", "pl_PL"];
	const language = new EnumField({
		name: "Language",
		keyName: "language_debug_sect",
		enumeration: available_languages,
		defaultValue: available_languages[0],
		style: {
			display: "flex",
			alignItems: "center",
		},
		callback: (value) => {
			const intValue = parseInt(value);
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

	const buttonContainer = createElement(
		"div",
		{
			key: "button_container",
			style: {
				background: "rgba(25,25,25,255)",
				width: "100%",
				height: "10%",
				display: "flex",
				bottom: "0",
				left: "0",
				position: "fixed",
				alignItems: "center",
			},
		},
		publishButton,
		process.env.NODE_ENV == "production" ? undefined : language.render(),
	);

	return createElement("div", {
		id: "publisher",
		style: {
			width: "100%",
			height: "100%",
			userSelect: "none",
			MozUserSelect: "none",
			KhtmlUserSelect: "none",
			WebkitUserSelect: "none",
		},
		children: [
			Header(),

			delim,
			BasicSettings(),
			SettingsMenu(),

			buttonContainer,
		],
	});
}

export function renderComponents(
	components: any[],
	condition: boolean = true,
): React.ReactNode[] {
	let result = components.map((component) => {
		if (component && component instanceof React.Component) {
			let result = component.render();
			if (component instanceof DisplayObject && component.IsAutoProperty) {
				const { useAutoProperties } = GetPublishContext();
				if (useAutoProperties) {
					return null;
				}
			}

			return result;
		}

		return component;
	});

	if (condition) {
		return result;
	} else {
		return [];
	}
}

export default Publisher;
