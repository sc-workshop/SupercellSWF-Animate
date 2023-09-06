import { createElement } from "react";
import SubMenu from "../../Shared/SubMenu";
import Locale from "../../../Localization";
import EnumField from "../../Shared/EnumField";
import { Settings } from "../../../PublisherSettings";

export default function ExportsSettings() {
    const localizedExportsMode = [
        Locale.Get("TID_EXPORTS_ALL_MOVIES"),
        Locale.Get("TID_EXPORTS_ALL_UNUSED_MOVIES")
    ];

    const exportsMode = EnumField(
        Locale.Get("TID_SWF_SETTINGS_EXPORTS_MODE"),
        "exports_mode_select",
        localizedExportsMode,
        Settings.getParam("exportsMode"),
        {
            marginBottom: "6px"
        },
        value => (Settings.setParam("exportsMode", parseInt(value))),
    );

    return SubMenu(
        Locale.Get("TID_EXPORTS_LABEL"),
        "exports_settings",
        {
            marginBottom: "6px"
        },
        exportsMode
    )
}