import { Settings, type SWFType } from "Publisher/PublisherSettings";
import React, { useState } from "react";

interface ContextProps {
	backwardCompatibility: boolean;
	fileType: SWFType;
	externalTextureFiles: boolean;
	useAutoProperties: boolean;
}

interface ContextInterface {
	useBackwardCompatibility: boolean;
	toggleBackwardCompatibility: () => void;

	fileType: SWFType;
	setFileType: (type: SWFType) => void;

	useExternalTextureFiles: boolean;
	toggleExternalTextureFiles: () => void;

	useAutoProperties: boolean;
	toggleAutoProperties: () => void;
}

//@ts-expect-error
const Context = React.createContext<ContextInterface>(null);

export const CreatePublishAppContext = (
	props: ContextProps,
): ContextInterface => {
	const [backwardCompatibility, setBackwardCompatibility] = useState(
		props.backwardCompatibility,
	);
	const [fileType, setFileType] = useState(props.fileType);
	const [useExternalTextureFiles, setUseExternalTextureFiles] = useState(
		props.externalTextureFiles,
	);
	const [useAutoProperties, setAutoProperties] = useState(
		props.useAutoProperties,
	);

	const backwardCompatibilitySetter = () => {
		setBackwardCompatibility(backwardCompatibility == false);

		Settings.setParam("backwardCompatibility", backwardCompatibility == false);
	};

	const fileTypeSetter = (type: SWFType) => {
		setFileType(type);
		Settings.setParam("type", type);
	};

	const externalTextureFileSetter = () => {
		setUseExternalTextureFiles(useExternalTextureFiles == false);

		Settings.setParam(
			"hasExternalTextureFile",
			useExternalTextureFiles == false,
		);
	};

	const autoPropertiesSetter = () => {
		setAutoProperties(useAutoProperties == false);

		Settings.setParam("autoProperties", useAutoProperties == false);
	};

	return {
		useBackwardCompatibility: backwardCompatibility,
		toggleBackwardCompatibility: backwardCompatibilitySetter,

		fileType: fileType,
		setFileType: fileTypeSetter,

		useExternalTextureFiles: useExternalTextureFiles,
		toggleExternalTextureFiles: externalTextureFileSetter,

		useAutoProperties: useAutoProperties,
		toggleAutoProperties: autoPropertiesSetter,
	};
};

export function UpdateContext() {
	const {
		useBackwardCompatibility,
		toggleBackwardCompatibility,
		setFileType,
		useExternalTextureFiles,
		toggleExternalTextureFiles,
		useAutoProperties,
		toggleAutoProperties,
	} = GetPublishContext();

	setFileType(Settings.getParam("type"));
	if (Settings.getParam("backwardCompatibility") != useBackwardCompatibility) {
		toggleBackwardCompatibility();
	}

	if (Settings.getParam("hasExternalTextureFile") != useExternalTextureFiles) {
		toggleExternalTextureFiles();
	}

	if (Settings.getParam("autoProperties") != useAutoProperties) {
		toggleAutoProperties();
	}
}

//@ts-expect-error
export const PublisherContextProvider = ({ children, ...props }) => {
	const context = CreatePublishAppContext(props as ContextProps);

	return <Context.Provider value={context}>{children}</Context.Provider>;
};

export function GetPublishContext() {
	const context = React.useContext(Context);
	if (!context) throw new Error("Failed to get context");
	return context;
}
