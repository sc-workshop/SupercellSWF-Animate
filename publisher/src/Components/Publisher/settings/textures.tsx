import Locale from "../../../Localization";
import { Settings, TextureDimensions, TextureEncoding, TextureScaleFactor } from "../../../PublisherSettings";

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
    const [textureEncodingMethod, setTextureEncodingMethod] = useState<TextureEncoding>(Settings.getParam("textureEncoding"));

    const exportToExternal = BoolField(
        Locale.Get("TID_SWF_SETTINGS_HAS_TEXTURE"),
        "external_texture_select",
        Settings.getParam("hasExternalTexture"),
        {
            marginBottom: "6px"
        },
        value => (Settings.setParam("hasExternalTexture", value)),
    );

    const textureEncoding = EnumField(
        Locale.Get("TID_SWF_SETTINGS_TEXTURE_ENCODING"),
        "texture_encoding_method",
        TextureEncoding,
        Settings.getParam("textureEncoding"),
        {
            marginBottom: "6px"
        },
        value => {
            const intValue = parseInt(value);
            setTextureEncodingMethod(intValue);
            Settings.setParam("textureEncoding", intValue);
        },
    );

    const textureQuality = EnumField(
        Locale.Get("TID_SWF_SETTINGS_TEXTURE_QUALITY"),
        "texture_quality_method",
        LocalizedTextureQuality,
        Settings.getParam("textureQuality"),
        {
            marginBottom: "6px"
        },
        value => (Settings.setParam("textureQuality", parseInt(value))),
    );

    const scaleFactor = EnumField(
        Locale.Get("TID_SWF_SETTINGS_SCALE_FACTOR"),
        "sprite_scale_factor_select",
        TextureScaleFactor,
        Settings.getParam("textureScaleFactor"),
        {
            marginBottom: "6px"
        },
        value => (Settings.setParam("textureScaleFactor", parseInt(value))),
    );

    const textureWidth = EnumField(
        Locale.Get("TID_SWF_SETTINGS_MAX_TEXTURE_WIDTH"),
        "texture_width_select",
        TextureDimensions,
        TextureDimensions.indexOf(Settings.getParam("textureMaxWidth")) as any,
        {
            marginBottom: "6px"
        },
        value => (Settings.setParam("textureMaxWidth", TextureDimensions[value as any])),
    )

    const textureHeight = EnumField(
        Locale.Get("TID_SWF_SETTINGS_MAX_TEXTURE_HEIGHT"),
        "texture_height_select",
        TextureDimensions,
        TextureDimensions.indexOf(Settings.getParam("textureMaxHeight")) as any,
        {
            //marginBottom: "6px"
        },
        value => (Settings.setParam("textureMaxHeight", TextureDimensions[value as any])),
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