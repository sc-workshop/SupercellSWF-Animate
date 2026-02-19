import BoolField from "Components/Shared/BoolField";
import EnumField from "Components/Shared/EnumField";
import SubMenu from "Components/Shared/SubMenu";
import { GetPublishContext } from "Publisher/Context";
import Locale from "Publisher/Localization";
import renderComponents from "Components/Shared/ComponentRenderer";
import {
	BaseCompressionMethods,
	CompressionMethods,
	Settings,
	SWFType,
} from "Publisher/PublisherSettings";

export default function OtherSettings() {
	const { fileType, useBackwardCompatibility, useAutoProperties } =
		GetPublishContext();

	const default_style = {
		display: "flex",
		alignItems: "center",
		//marginBottom: "10px",
	};

	const compressionType = new EnumField({
		name: Locale.Get("TID_SWF_SETTINGS_COMPRESSION"),
		keyName: "file_compression_select",
		enumeration: useBackwardCompatibility
			? BaseCompressionMethods
			: CompressionMethods,
		defaultValue: Settings.getParam("compressionMethod"),
		style: default_style,
		callback: (value) =>
			Settings.setParam("compressionMethod", parseInt(value, 10)),
	});
	compressionType.IsAutoProperty = true;

	const precisionMatrix = new BoolField({
		name: Locale.Get("TID_SWF_SETTINGS_PRECISION_MATRIX"),
		keyName: "precision_matrix",
		defaultValue: Settings.getParam("hasPrecisionMatrices"),
		style: default_style,
		callback: (value) => Settings.setParam("hasPrecisionMatrices", value),
	});

	const writeCustomProperties = new BoolField({
		name: Locale.Get("TID_SWF_WRITE_CUSTOM_PROPERTIES"),
		keyName: "custom_properties",
		defaultValue: Settings.getParam("writeCustomProperties"),
		style: default_style,
		callback: (value) => Settings.setParam("writeCustomProperties", value),
	});
	writeCustomProperties.IsAutoProperty = true;

	const writeFieldsText = new BoolField({
		name: Locale.Get("TID_SWF_WRITE_FIELDS_TEXT"),
		keyName: "fields_text",
		defaultValue: Settings.getParam("writeFieldsText"),
		style: default_style,
		callback: (value) => Settings.setParam("writeFieldsText", value),
	});

	const aggressiveRasterize = new BoolField({
		name: Locale.Get("TID_SWF_SETTINGS_AGGRESSIVE_RASTERIZE"),
		keyName: "aggressive_rasterization",
		defaultValue: Settings.getParam("aggressiveRasterization"),
		style: default_style,
		callback: (value) => Settings.setParam("aggressiveRasterization", value),
		tip_tid: "TID_SWF_SETTINGS_AGGRESSIVE_RASTERIZE_TIP",
	});

	if (useBackwardCompatibility) {
		Settings.setParam("hasPrecisionMatrices", false);
		precisionMatrix.state.checked = false;

		Settings.setParam("writeCustomProperties", false);
		writeCustomProperties.state.checked = false;

		Settings.setParam("compressionMethod", CompressionMethods.LZMA);
	}

	const backwardCompatibilityProps = renderComponents(
		[writeCustomProperties, precisionMatrix],
		!useBackwardCompatibility,
	);

	const sc1Props = renderComponents(
		[compressionType, backwardCompatibilityProps],
		fileType == SWFType.SC1 || useAutoProperties,
	);

	const props = renderComponents([
		writeFieldsText,
		aggressiveRasterize,
		sc1Props,
	]);

	return SubMenu(
		Locale.Get("TID_OTHER_LABEL"),
		"other_settings",
		{
			marginBottom: "20%",
		},
		props,
	);
}
