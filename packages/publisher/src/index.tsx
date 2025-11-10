import { CSEvent, getInterface, isCEP } from "CEP";
import { PublisherContextProvider } from "Publisher/Context";
import Locale from "Publisher/Localization";
import Publisher from "Publisher/Publisher";
import { Settings } from "Publisher/PublisherSettings";
import { createElement, useEffect, useState } from "react";
import ReactDOM from "react-dom/client";

export const loadFont = async (callback: () => void) => {
	const font = new FontFace(
		"PublisherFont",
		`url(${require(`./Assets/fonts/${Locale.code}`)})`,
		{ style: "normal" },
	);
	await font.load();
	if (font.status === "loaded") {
		document.fonts.add(font);
	}
	callback();
};

function App() {
	const [publisherStateData, setPublisherStateData] = useState<
		string | undefined
	>(undefined);
	const [isFontLoaded, setIsFontLoaded] = useState(false);
	const [visible, setVisible] = useState(false);

	useEffect(() => {
		loadFont(() => {
			setIsFontLoaded(true);
		});

		// Publisher Data
		if (!isCEP()) {
			setPublisherStateData("{}");
			return;
		}

		const CSInterface = getInterface();
		const getData = async () => {
			const publisherData = new Promise<void>((resolve) => {
				CSInterface.addEventListener(
					"com.adobe.events.flash.extension.setstate",
					(event: CSEvent) => {
						if (event.data === undefined) {
							setPublisherStateData("{}");
						} else {
							setPublisherStateData(event.data);
						}
						resolve();
					},
				);
			});

			const event = new CSEvent(
				"com.adobe.events.flash.extensionLoaded",
				"APPLICATION",
				undefined,
				"org.scWorkshop.SupercellSWF.PublishSettings",
			);
			event.data = "SupercellSWF publisher loaded";
			CSInterface.dispatchEvent(event);

			await publisherData;
		};
		getData();
	}, []);

	useEffect(() => {
		if (publisherStateData !== undefined && isFontLoaded) {
			const timeout = setTimeout(() => setVisible(true), 50);
			return () => clearTimeout(timeout);
		}
	}, [publisherStateData, isFontLoaded]);

	if (publisherStateData !== undefined && isFontLoaded) {
		Settings.restore(publisherStateData);

		return createElement(
			"body",
			{
				key: "publisher_body",
				style: {
					fontFamily: "PublisherFont",
					backgroundColor: "#333333",
					position: "relative",
					opacity: visible ? 1 : 0,
					transition: "opacity 1s ease",
				},
			},
			<PublisherContextProvider
				backwardCompatibility={false}
				externalTextureFiles={false}
				useAutoProperties={false}
			>
				<Publisher />
			</PublisherContextProvider>,
		);
	} else {
		return null;
	}
}

const root = ReactDOM.createRoot(
	document.getElementById("root") as HTMLElement,
);
root.render(<App />);
