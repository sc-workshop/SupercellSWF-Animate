import { Enumfield } from "../../Shared/Enumfield";
import { Locale } from "../../../Localization";
import { State } from "../../publisherState";
import { DropdowMenu } from "../../Shared/Dropdown";
import { CompressionMethods } from "../../publisherState"
import { TextureSettings } from "./textures";

export function Settings() {
    const compressionType = Enumfield(
        Locale.Get("TID_SWF_SETTINGS_COMPRESSION"),
        CompressionMethods,
        State.getParam("compression"),
        {
            marginBottom: "10px"
        },
        value => (State.setParam("compression", CompressionMethods[value as any])),
    )

    return DropdowMenu(
        Locale.Get("TID_ADDITIONAL_SETTINGS_LABEL"),
        {
            marginBottom: "12px"
        },
        compressionType,
        TextureSettings()
    )
}