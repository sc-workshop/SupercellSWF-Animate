import Locale from "Publisher/Localization";
import { Settings, SWFType, TextureDimensions, TextureEncoding, TextureScaleFactor } from "Publisher/PublisherSettings";

import BoolField from "Components/Shared/BoolField";
import SubMenu from "Components/Shared/SubMenu";
import EnumField from "Components/Shared/EnumField";

import { useState } from "react"
import StringField from "Components/Shared/StringField";
import { GetPublishContext } from "Publisher/Context";
import { renderComponents } from "Publisher/Publisher";

const LocalizedTextureQuality = [
    Locale.Get("TID_HIGHEST"),
    Locale.Get("TID_HIGH"),
    Locale.Get("TID_MEDIUM"),
    Locale.Get("TID_LOW"),
]

export default function TextureSettings() {
    const [textureEncodingMethod, setTextureEncodingMethod] = useState<TextureEncoding>(Settings.getParam("textureEncoding"));
    const [useMultiresTexture, setUseMultiresTexture] = useState<boolean>(Settings.getParam("hasMultiresTexture"));
    const [useLowresTexture, setUseLowresTexture] = useState<boolean>(Settings.getParam("hasLowresTexture"));
    const { useBackwardCompatibility, fileType, useAutoProperties, toggleExternalTextureFiles } = GetPublishContext();

    if (useBackwardCompatibility) {
        Settings.setParam("textureEncoding", TextureEncoding.Raw);
    } else {
        Settings.setParam("textureEncoding", textureEncodingMethod)
    }

    let encoding = Settings.getParam("textureEncoding");
    console.log(encoding)

    const defaultStyle =
    {
        display: "flex",
        alignItems: "center",
        marginBottom: "10px"
    };

    const exportToExternal = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_HAS_TEXTURE"),
            keyName: "external_texture_select",
            defaultValue: Settings.getParam("hasExternalTexture"),
            style: defaultStyle,
            callback: value => (Settings.setParam("hasExternalTexture", value)),
            tip_tid: "TID_SWF_SETTINGS_HAS_TEXTURE_TIP"
        }
    );
    exportToExternal.IsAutoProperty = true;

    const textureEncoding = new EnumField({
        name: Locale.Get("TID_SWF_SETTINGS_TEXTURE_ENCODING"),
        keyName: "texture_encoding_method",
        enumeration: TextureEncoding,
        defaultValue: Settings.getParam("textureEncoding"),
        style: defaultStyle,
        callback: value => {
            const intValue = parseInt(value);
            setTextureEncodingMethod(intValue);
            Settings.setParam("textureEncoding", intValue);
        },
        tip_tid: "TID_SWF_SETTINGS_TEXTURE_ENCODING_TIP"
    });
    textureEncoding.IsAutoProperty = true;

    const textureExportExernalFile = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_HAS_EXTERNAL_TEXTURE_FILE"),
            keyName: "external_texture_file_select",
            defaultValue: Settings.getParam("hasExternalTextureFile"),
            style: defaultStyle,
            callback: toggleExternalTextureFiles,
            tip_tid: "TID_SWF_SETTINGS_HAS_EXTERNAL_TEXTURE_FILE_TIP"
        }
    );
    textureExportExernalFile.IsAutoProperty = true;

    const textureCompressExternalFile = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_COMPRESS_EXTERNAL_TEXTURE_FILE"),
            keyName: "external_compressed_texture_file_select",
            defaultValue: Settings.getParam("compressExternalTextureFile"),
            style: defaultStyle,
            callback: value => (Settings.setParam("compressExternalTextureFile", value)),
            tip_tid: "TID_SWF_SETTINGS_COMPRESS_EXTERNAL_TEXTURE_FILE_TIP"
        }
    );

    const textureQuality = new EnumField({
        name: Locale.Get("TID_SWF_SETTINGS_TEXTURE_QUALITY"),
        keyName: "texture_quality_method",
        enumeration: LocalizedTextureQuality,
        defaultValue: Settings.getParam("textureQuality"),
        style: defaultStyle,
        callback: value => (Settings.setParam("textureQuality", parseInt(value))),
    });

    const useMultiresTextures = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_HAS_MULTIRES_TEXTURES"),
            keyName: "use_multires_textures_select",
            defaultValue: Settings.getParam("hasMultiresTexture"),
            style: defaultStyle,
            callback: value => {

                setUseMultiresTexture(value);
                Settings.setParam("hasMultiresTexture", value);
            },
            tip_tid: "TID_SWF_SETTINGS_HAS_MULTIRES_TEXTURES_TIP"
        }
    );
    useMultiresTextures.IsAutoProperty = true;

    const multiresSuffix = StringField(
        Locale.Get("TID_SWF_SETTINGS_HAS_MULTIRES_TEXTURES_SUFFIX"),
        "multires_suffix",
        defaultStyle,
        value => (Settings.setParam("multiResolutinSuffix", value)),
        Settings.getParam("multiResolutinSuffix"),
    );

    const useLowresTextures = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_HAS_LOWRES_TEXTURES"),
            keyName: "use_lowres_textures_select",
            defaultValue: Settings.getParam("hasLowresTexture"),
            style: defaultStyle,
            callback: value => {
                setUseLowresTexture(value);
                Settings.setParam("hasLowresTexture", value)
            },
            tip_tid: "TID_SWF_SETTINGS_HAS_LOWRES_TEXTURES_TIP"
        }
    );
    useLowresTextures.IsAutoProperty = true;

    const generateLowresTextures = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_GENERATE_LOWRES_TEXTURES"),
            keyName: "generate_lowres_textures_select",
            defaultValue: Settings.getParam("generateLowresTexture"),
            style: defaultStyle,
            callback: value => (Settings.setParam("generateLowresTexture", value)),
            tip_tid: "TID_SWF_SETTINGS_GENERATE_LOWRES_TEXTURES_TIP"
        }
    );
    generateLowresTextures.IsAutoProperty = true;

    const generateStreamingTexture = new BoolField(
        {
            name: Locale.Get("TID_SWF_SETTINGS_GENERATE_STREAMING_TEXTURE"),
            keyName: "generate_streaming_texture_select",
            defaultValue: Settings.getParam("generateStreamingTexture"),
            style: defaultStyle,
            callback: value => (Settings.setParam("generateStreamingTexture", value)),
            tip_tid: "TID_SWF_SETTINGS_GENERATE_STREAMING_TEXTURE_TIP"
        }
    );
    generateStreamingTexture.IsAutoProperty = true;

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
    });

    const textureWidth = new EnumField({
        name: Locale.Get("TID_SWF_SETTINGS_MAX_TEXTURE_WIDTH"),
        keyName: "texture_width_select",
        enumeration: TextureDimensions,
        defaultValue: TextureDimensions.indexOf(Settings.getParam("textureMaxWidth")) as never,
        style: {
            display: "flex",
            alignItems: "center",
            marginBottom: "10px"
        },
        callback: value => (Settings.setParam("textureMaxWidth", TextureDimensions[value as never])),
    });

    const textureHeight = new EnumField({
        name: Locale.Get("TID_SWF_SETTINGS_MAX_TEXTURE_HEIGHT"),
        keyName: "texture_height_select",
        enumeration: TextureDimensions,
        defaultValue: TextureDimensions.indexOf(Settings.getParam("textureMaxHeight")) as never,
        style: {
            display: "flex",
            alignItems: "center",
            marginBottom: "10px"
        },
        callback: value => (Settings.setParam("textureMaxHeight", TextureDimensions[value as never])),
    });

    let multiresProps = renderComponents([multiresSuffix, lowresSuffix], useMultiresTexture);
    let lowresProps = renderComponents([generateLowresTextures], useLowresTexture);
    let props = renderComponents([textureEncoding], !useBackwardCompatibility);
    let backwardCompatibilityProps = renderComponents([textureQuality], useBackwardCompatibility);

    let resolutionProps = renderComponents([
        useLowresTextures,
        useMultiresTextures,
        ...multiresProps,
        ...lowresProps,
    ],
        encoding != TextureEncoding.SCTX // SCTX has streaming textures and mip maps so there is no point in using lowres textures
    )

    let rawTextureProps = renderComponents(
        backwardCompatibilityProps,
        encoding == TextureEncoding.Raw
    );

    let generalCompressedTextureProps = renderComponents(
        [textureCompressExternalFile, textureExportExernalFile],
        (encoding == TextureEncoding.KTX || encoding == TextureEncoding.SCTX) || useAutoProperties
    )

    let supercellTextureProps = renderComponents(
        [generateStreamingTexture],
        encoding == TextureEncoding.SCTX
    )

    let generalProps = renderComponents([
        ...generalCompressedTextureProps,
        scaleFactor,
        textureWidth,
        textureHeight
    ])

    let advancedVersionDependentProps = renderComponents([
        exportToExternal
    ], fileType == SWFType.SC1);

    let advancedProps = renderComponents([
        ...props,
        ...rawTextureProps,
        ...supercellTextureProps,
        ...advancedVersionDependentProps,
        ...resolutionProps
    ], !useAutoProperties);

    return SubMenu(
        Locale.Get("TID_TEXTURES_LABEL"),
        "texture_settings",
        {
            marginBottom: "6px"
        },
        ...advancedProps,
        ...generalProps,
    )
}