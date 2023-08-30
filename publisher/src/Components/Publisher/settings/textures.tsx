import Locale from "../../../Localization";
import { State, TextureDimensions, TextureEncoding, TextureScaleFactor } from "../../publisherState";

import BoolField from "../../Shared/BoolField";
import SubMenu from "../../Shared/SubMenu";
import EnumField from "../../Shared/EnumField";

import { useState } from "react"

const LocalizedTextureQuality = [
    Locale.Get("TID_HIGHEST"),
    Locale.Get("TID_HIGH"),
    Locale.Get("TID_MEDIUM"),
    Locale.Get("TID_LOW"),
]

export default function TextureSettings() {
    const [textureEncodingMethod, setTextureEncodingMethod] = useState<TextureEncoding>(State.getParam("textureEncoding"));

    const exportToExternal = BoolField(
        Locale.Get("TID_SWF_SETTINGS_HAS_TEXTURE"),
        "external_texture_select",
        State.getParam("hasExternalTexture"),
        {
            marginBottom: "6px"
        },
        value => (State.setParam("hasExternalTexture", value)),
    );

    const textureEncoding = EnumField(
        Locale.Get("TID_SWF_SETTINGS_TEXTURE_ENCODING"),
        "texture_encoding_method",
        TextureEncoding,
        State.getParam("textureEncoding"),
        {
            marginBottom: "6px"
        },
        value => {
            const intValue = parseInt(value);
            setTextureEncodingMethod(intValue);
            State.setParam("textureEncoding", intValue);
        },
    );

    const textureQuality = EnumField(
        Locale.Get("TID_SWF_SETTINGS_TEXTURE_QUALITY"),
        "texture_quality_method",
        LocalizedTextureQuality,
        State.getParam("textureQuality"),
        {
            marginBottom: "6px"
        },
        value => (State.setParam("textureQuality", parseInt(value))),
    );

    const scaleFactor = EnumField(
        Locale.Get("TID_SWF_SETTINGS_SCALE_FACTOR"),
        "sprite_scale_factor_select",
        TextureScaleFactor,
        State.getParam("textureScaleFactor"),
        {
            marginBottom: "6px"
        },
        value => (State.setParam("textureScaleFactor", parseInt(value))),
    );

    const textureWidth = EnumField(
        Locale.Get("TID_SWF_SETTINGS_MAX_TEXTURE_WIDTH"),
        "texture_width_select",
        TextureDimensions,
        TextureDimensions.indexOf(State.getParam("textureMaxWidth")) as any,
        {
            marginBottom: "6px"
        },
        value => (State.setParam("textureMaxWidth", TextureDimensions[value as any])),
    )

    const textureHeight = EnumField(
        Locale.Get("TID_SWF_SETTINGS_MAX_TEXTURE_HEIGHT"),
        "texture_height_select",
        TextureDimensions,
        TextureDimensions.indexOf(State.getParam("textureMaxHeight")) as any,
        {
            //marginBottom: "6px"
        },
        value => (State.setParam("textureMaxHeight", TextureDimensions[value as any])),
    )

    return SubMenu(
        Locale.Get("TID_TEXTURES_LABEL"),
        "texture_settings",
        {
            marginBottom: "6px"
        },
        exportToExternal,
        textureEncoding,
        textureEncodingMethod == TextureEncoding.Raw ? textureQuality : undefined,
        scaleFactor,
        textureWidth,
        textureHeight
    )
}