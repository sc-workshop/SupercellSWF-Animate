import Locale from "../../../../Localization";
import { Settings, TextureDimensions, TextureEncoding, TextureScaleFactor } from "../../../../PublisherSettings";

import BoolField from "../../../Shared/BoolField";
import SubMenu from "../../../Shared/SubMenu";
import EnumField from "../../../Shared/EnumField";

import { ReactNode, useState } from "react"
import StringField from "../../../Shared/StringField";
import { GetPublishContext } from "../../../../Context";

const LocalizedTextureQuality = [
    Locale.Get("TID_HIGHEST"),
    Locale.Get("TID_HIGH"),
    Locale.Get("TID_MEDIUM"),
    Locale.Get("TID_LOW"),
]

export default function TextureSettings() {
    const [textureEncodingMethod, setTextureEncodingMethod] = useState<TextureEncoding>(Settings.getParam("textureEncoding"));
    const [useMultiresStatus, setUseMultiresStatus] = useState<boolean>(Settings.getParam("hasMultiresTexture"));
    const { useBackwardCompatibility } = GetPublishContext();

    if (useBackwardCompatibility) {
        Settings.setParam("textureEncoding", TextureEncoding.Raw);
    } else 
    {
        Settings.setParam("textureEncoding", textureEncodingMethod)
    }

    const exportToExternal = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_HAS_TEXTURE"),
            keyName: "external_texture_select",
            defaultValue: Settings.getParam("hasExternalTexture"),
            style: {
                display: "flex",
                alignItems: "center",
                marginBottom: "10px"
            },
            callback: value => (Settings.setParam("hasExternalTexture", value)),
            tip_tid: "TID_SWF_SETTINGS_HAS_TEXTURE_TIP"
        }
    ).render();

    const textureEncoding = new EnumField({
        name: Locale.Get("TID_SWF_SETTINGS_TEXTURE_ENCODING"),
        keyName: "texture_encoding_method",
        enumeration: TextureEncoding,
        defaultValue: Settings.getParam("textureEncoding"),
        style: {
            display: "flex",
            alignItems: "center",
            marginBottom: "10px"
        },
        callback: value => {
            const intValue = parseInt(value);
            setTextureEncodingMethod(intValue);
            Settings.setParam("textureEncoding", intValue);
        },
        tip_tid: "TID_SWF_SETTINGS_TEXTURE_ENCODING_TIP"
    }
    ).render();

    const textureCompressedExternal = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_HAS_EXTERNAL_COMPRESSED_TEXTURE"),
            keyName: "external_compressed_texture_select",
            defaultValue: Settings.getParam("hasExternalTexture"),
            style: {
                display: "flex",
                alignItems: "center",
                marginBottom: "10px"
            },
            callback: value => (Settings.setParam("hasExternalTexture", value)),
            tip_tid: "TID_SWF_SETTINGS_HAS_EXTERNAL_COMPRESSED_TEXTURE_TIP"
        }
    ).render();


    const textureQuality = new EnumField({
        name: Locale.Get("TID_SWF_SETTINGS_TEXTURE_QUALITY"),
        keyName: "texture_quality_method",
        enumeration: LocalizedTextureQuality,
        defaultValue: Settings.getParam("textureQuality"),
        style: {
            display: "flex",
            alignItems: "center",
            marginBottom: "10px"
        },
        callback: value => (Settings.setParam("textureQuality", parseInt(value))),
    }).render();

    const useMultiresTextures = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_HAS_MULTIRES_TEXTURES"),
            keyName: "use_multires_textures_select",
            defaultValue: Settings.getParam("hasMultiresTexture"),
            style: {
                display: "flex",
                alignItems: "center",
                marginBottom: "10px"
            },
            callback: value => {

                setUseMultiresStatus(value);
                Settings.setParam("hasMultiresTexture", value);
            },
            tip_tid: "TID_SWF_SETTINGS_HAS_MULTIRES_TEXTURES_TIP"
        }
    ).render();

    const multiresSuffix = StringField(
        Locale.Get("TID_SWF_SETTINGS_HAS_MULTIRES_TEXTURES_SUFFIX"),
        "multires_suffix",
        {
            display: "flex",
            alignItems: "center",
            marginBottom: "10px"
        },
        value => (Settings.setParam("multiResolutinSuffix", value)),
        Settings.getParam("multiResolutinSuffix"),
    );

    const useLowresTextures = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_HAS_LOWRES_TEXTURES"),
            keyName: "use_lowres_textures_select",
            defaultValue: Settings.getParam("hasLowresTexture"),
            style: {
                display: "flex",
                alignItems: "center",
                marginBottom: "10px"
            },
            callback: value => (Settings.setParam("hasLowresTexture", value)),
            tip_tid: "TID_SWF_SETTINGS_HAS_LOWRES_TEXTURES_TIP"
        }
    ).render();

    const lowresSuffix = StringField(
        Locale.Get("TID_SWF_SETTINGS_HAS_LOWRES_TEXTURES_SUFFIX"),
        "lowres_suffix",
        {
            display: "flex",
            alignItems: "center",
            marginBottom: "10px"
        },
        value => (Settings.setParam("lowResolutionSuffix", value)),
        Settings.getParam("lowResolutionSuffix"),
    );

    const scaleFactor = new EnumField({
        name: Locale.Get("TID_SWF_SETTINGS_SCALE_FACTOR"),
        keyName: "sprite_scale_factor_select",
        enumeration: TextureScaleFactor,
        defaultValue: Settings.getParam("textureScaleFactor"),
        style: {
            display: "flex",
            alignItems: "center",
            marginBottom: "10px"
        },
        callback: value => (Settings.setParam("textureScaleFactor", parseInt(value))),
        tip_tid: "TID_SWF_SETTINGS_SCALE_FACTOR_TIP"
    }).render();

    const textureWidth = new EnumField({
        name: Locale.Get("TID_SWF_SETTINGS_MAX_TEXTURE_WIDTH"),
        keyName: "texture_width_select",
        enumeration: TextureDimensions,
        defaultValue: TextureDimensions.indexOf(Settings.getParam("textureMaxWidth")) as any,
        style: {
            display: "flex",
            alignItems: "center",
            marginBottom: "10px"
        },
        callback: value => (Settings.setParam("textureMaxWidth", TextureDimensions[value as any])),
    }).render();

    const textureHeight = new EnumField({
        name: Locale.Get("TID_SWF_SETTINGS_MAX_TEXTURE_HEIGHT"),
        keyName: "texture_height_select",
        enumeration: TextureDimensions,
        defaultValue: TextureDimensions.indexOf(Settings.getParam("textureMaxHeight")) as any,
        style: {
            display: "flex",
            alignItems: "center",
            marginBottom: "10px"
        },
        callback: value => (Settings.setParam("textureMaxHeight", TextureDimensions[value as any])),
    }).render();

    let texture_props: ReactNode[] = []

    if (useBackwardCompatibility) {
        texture_props = [textureQuality];
    } else {
        switch (textureEncodingMethod) {
            case TextureEncoding.Raw:
                texture_props = [textureQuality];
                break;
            case TextureEncoding.KTX:
                texture_props = [textureCompressedExternal]
                break;
        }
    }


    return SubMenu(
        Locale.Get("TID_TEXTURES_LABEL"),
        "texture_settings",
        {
            marginBottom: "6px"
        },
        exportToExternal,
        !useBackwardCompatibility ? textureEncoding : undefined,
        ...texture_props,
        useMultiresTextures,
        useMultiresStatus ? multiresSuffix : undefined,
        useMultiresStatus ? lowresSuffix : undefined,
        useLowresTextures,
        scaleFactor,
        textureWidth,
        textureHeight
    )
}