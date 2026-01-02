import ElementsArray from "Components/Shared/Array";
import renderComponents from "Components/Shared/ComponentRenderer";
import { GetPublishContext } from "Publisher/Context";
import Locale from "Publisher/Localization";
import { Settings, SWFType } from "Publisher/PublisherSettings";
import type React from "react";
import { createElement, useState } from "react";
import BoolField from "../Shared/BoolField";
import EnumField from "../Shared/EnumField";
import FileField from "../Shared/FileField";

export default function BasicSettings() {
	const {
		setFileType,
		useAutoProperties,
		useBackwardCompatibility,
		toggleBackwardCompatibility,
		toggleAutoProperties,
	} = GetPublishContext();
	const [isExportToExternal, setExportToExternal] = useState(
		Settings.getParam("exportToExternal"),
	);

	const [useDocuments, setUseDocuments] = useState(
		Settings.getParam("multipleDocuments"),
	);

	const [documentPaths] = useState<Map<number, string>>(new Map());

	const defaultStyle = {
		display: "flex",
		alignItems: "center",
	};

	const output = FileField({
		name: Locale.Get("TID_OUTPUT"),
		keyName: "publisher_output_path",
		mode: "write",
		ext: "sc",
		style: defaultStyle,
		callback: (value) => Settings.setParam("output", value),
		defaultValue: Settings.getParam("output"),
	});

	const filetype = new EnumField({
		name: Locale.Get("TID_SWF_SETTINGS_FILETYPE"),
		keyName: "swf_type",
		enumeration: SWFType,
		defaultValue: Settings.getParam("type"),
		style: defaultStyle,
		callback: (value) => {
			const type: SWFType = parseInt(value, 10) as SWFType;
			setFileType(type);

			if (type == SWFType.SC2 && useBackwardCompatibility)
				toggleBackwardCompatibility();
		},
	});
	filetype.IsAutoProperty = true;

	const exportToExternal = new BoolField({
		name: Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL"),
		keyName: "export_to_external_select",
		defaultValue: Settings.getParam("exportToExternal"),
		style: defaultStyle,
		callback: (value) => {
			setExportToExternal(value);
			if (!value && useAutoProperties) toggleAutoProperties();
		},
		tip_tid: "TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL_TIP",
	});
	Settings.data.exportToExternal = isExportToExternal;

	const externalFileOptionsStyle = {
		display: "flex",
		alignItems: "center",
		marginLeft: "2%",
	};

	const externalFilePath = FileField({
		name: Locale.Get("TID_SWF_SETTINGS_EXPORT_TO_EXTERNAL_PATH"),
		keyName: "export_to_external_path",
		mode: "read",
		ext: "sc",
		style: externalFileOptionsStyle,
		callback: (value) => {
			Settings.setParam("exportToExternalPath", value);
		},
		defaultValue: Settings.getParam("exportToExternalPath"),
	});

	const autoSettings = new BoolField({
		name: Locale.Get("TID_SWF_SETTINGS_AUTO"),
		keyName: "auto_settings_select",
		defaultValue: Settings.getParam("autoProperties"),
		style: externalFileOptionsStyle,
		callback: () => {
			toggleAutoProperties();
		},
		tip_tid: "TID_SWF_SETTINGS_AUTO_TIP",
	});

	const repackAtlas = new BoolField({
		name: Locale.Get("TID_SWF_REPACK_ATLAS"),
		keyName: "repack_atlas_select",
		defaultValue: Settings.getParam("repackAtlas"),
		style: externalFileOptionsStyle,
		callback: (value) => Settings.setParam("repackAtlas", value),
		tip_tid: "TID_SWF_REPACK_ATLAS_TIP",
	});

	const multipleDocuments = new BoolField({
		name: Locale.Get("TID_SWF_MULTIPLE_DOCUMENTS"),
		keyName: "multiple_documents_select",
		defaultValue: Settings.getParam("multipleDocuments"),
		style: defaultStyle,
		callback: setUseDocuments,
		tip_tid: "TID_SWF_MULTIPLE_DOCUMENTS_TIP",
	});
	Settings.data.multipleDocuments = useDocuments;

	const flushDocuments = () => {
		const files = Array.from(documentPaths.values());
		Settings.setParam("documentsPaths", files);
	};

	const changeDocument = (id: number, path: string) => {
		documentPaths.set(id, path);
		flushDocuments();
	};

	const createDocument = (id: number, path: string = "") => {
		const result = (
			<FileField
				keyName={`document_path_${Date.now()}`}
				mode="read"
				ext="fla"
				defaultValue={path}
				style={defaultStyle}
				callback={(value) => changeDocument(id, value)}
			></FileField>
		);

		documentPaths.set(id, path);
		return result;
	};

	const deleteDocument = (id: number) => {
		documentPaths.delete(id);
		flushDocuments();
	};

	const documentArray = ElementsArray<React.JSX.Element, string>({
		name: Locale.Get("TID_SWF_DOCUMENTS_ARRAY"),
		keyName: "multiple_documents",
		style: {
			minWidth: "50%",
		},
		initValues: Settings.getParam("documentsPaths"),
		onCreate: createDocument,
		onRemove: deleteDocument,
	});

	const externalFileSettings = renderComponents(
		[externalFilePath, autoSettings, repackAtlas],
		isExportToExternal,
	);

	const multipleDocumentsSettings = renderComponents(
		[documentArray],
		useDocuments,
	);

	const props = renderComponents([
		output,
		filetype,
		exportToExternal,
		externalFileSettings,
		multipleDocuments,
		multipleDocumentsSettings,
	]);

	return createElement(
		"div",
		{
			key: "publisher_basic_settings",
			style: {
				width: "100%",
				marginBottom: "8px",
			},
		},
		props,
	);
}
