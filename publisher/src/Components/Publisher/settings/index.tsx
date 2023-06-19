
import Locale from "../../../Localization";
import { State } from "../../publisherState";
import { CompressionMethods } from "../../publisherState"

import SubMenu from "../../Shared/SubMenu";
import EnumField from "../../Shared/EnumField";
import TextureSettings from "./textures";

export default function Settings() {
    const compressionType = EnumField(
        Locale.Get("TID_SWF_SETTINGS_COMPRESSION"),
        "file_compression_select",
        CompressionMethods,
        CompressionMethods[State.getParam("compression")],
        {
            marginBottom: "6px"
        },
        value => (State.setParam("compression", CompressionMethods[value as any])),
    )

    return SubMenu(
        Locale.Get("TID_ADDITIONAL_SETTINGS_LABEL"),
        "additional_settings",
        {
            marginBottom: "12px"
        },
        compressionType,
        TextureSettings()
    )
}