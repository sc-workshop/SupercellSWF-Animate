import { Locale } from "../../../Localization";
import { Boolfield } from "../../Shared/Boolfield";
import { DropdowMenu } from "../../Shared/Dropdown";
import { Enumfield } from "../../Shared/Enumfield";
import { State, TextureDimensions, TextureScaleFactor } from "../../publisherState";

export function TextureSettings() {
    const exportToExternal = Boolfield(
        Locale.Get("TID_SWF_SETTINGS_HAS_TEXTURE"),
        State.getParam("hasTexture"),
        {
            marginBottom: "10px"
        },
        value => (State.setParam("hasTexture", value)),
    )

    const scaleFactor = Enumfield(
        Locale.Get("TID_SWF_SETTINGS_SCALE_FACTOR"),
        TextureScaleFactor,
        State.getParam("textureScaleFactor"),
        {
            marginBottom: "10px"
        },
        value => (State.setParam("textureScaleFactor", value)),
    )

    const textureWidth = Enumfield(
        Locale.Get("TID_SWF_SETTINGS_MAX_TEXTURE_WIDTH"),
        TextureDimensions,
        "2",
        {
            marginBottom: "10px"
        },
        value => (State.setParam("textureMaxWidth", TextureDimensions[value as any])),
    )

    const textureHeight = Enumfield(
        Locale.Get("TID_SWF_SETTINGS_MAX_TEXTURE_HEIGHT"),
        TextureDimensions,
        "2",
        {
            marginBottom: "10px"
        },
        value => (State.setParam("textureMaxHeight", TextureDimensions[value as any])),
    )

    return DropdowMenu(
        Locale.Get("TID_TEXTURES_LABEL"),
        {
            marginBottom: "12px"
        },
        exportToExternal,
        scaleFactor,
        textureWidth,
        textureHeight
    )
}