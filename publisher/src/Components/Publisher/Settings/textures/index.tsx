import Locale from "Localization";
import { Settings, SWFType, TextureDimensions, TextureEncoding, TextureScaleFactor } from "PublisherSettings";

import BoolField from "Components/Shared/BoolField";
import SubMenu from "Components/Shared/SubMenu";
import EnumField from "Components/Shared/EnumField";

import { Component, ReactNode, useState } from "react"
import StringField from "Components/Shared/StringField";
import { GetPublishContext } from "Context";
import { renderComponents } from "Publisher";
import { render } from "react-dom";

const LocalizedTextureQuality = [
    Locale.Get("TID_HIGHEST"),
    Locale.Get("TID_HIGH"),
    Locale.Get("TID_MEDIUM"),
    Locale.Get("TID_LOW"),
]

export default function TextureSettings() {
    const [textureEncodingMethod, setTextureEncodingMethod] = useState<TextureEncoding>(Settings.getParam("textureEncoding"));
    const [useMultiresStatus, setUseMultiresStatus] = useState<boolean>(Settings.getParam("hasMultiresTexture"));
    const [useLowresTexture, setUseLowresTexture] = useState<boolean>(Settings.getParam("hasLowresTexture"));
    const { useBackwardCompatibility, fileType, useExternalTextureFiles, toggleExternalTextureFiles } = GetPublishContext();

    if (useBackwardCompatibility) {
        Settings.setParam("textureEncoding", TextureEncoding.Raw);
    } else 
    {
        Settings.setParam("textureEncoding", textureEncodingMethod)
    }

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
    }
    );

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

    const textureCompressExternalFIle = new BoolField(
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

                setUseMultiresStatus(value);
                Settings.setParam("hasMultiresTexture", value);
            },
            tip_tid: "TID_SWF_SETTINGS_HAS_MULTIRES_TEXTURES_TIP"
        }
    );

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

    let generalProps = renderComponents([
        useLowresTextures,
        scaleFactor,
        textureWidth,
        textureHeight
    ])

    let encodingDependentProps: any[] = []

    if (useBackwardCompatibility) {
        encodingDependentProps = renderComponents([textureQuality]);
    } else {
        switch (textureEncodingMethod) {
            case TextureEncoding.Raw:
                encodingDependentProps = renderComponents([textureQuality]);
                break;
            case TextureEncoding.KTX:
                encodingDependentProps = renderComponents([
                    textureExportExernalFile, 
                    renderComponents([textureCompressExternalFIle], useExternalTextureFiles)
                ]);
                break;
        }
    }

    let versionDependentProps = renderComponents([
         exportToExternal,
            !useBackwardCompatibility ? textureEncoding : undefined,
            ...encodingDependentProps, 
            useMultiresTextures,
            useMultiresStatus ? multiresSuffix : undefined,
            useMultiresStatus ? lowresSuffix : undefined,
            ...renderComponents([generateLowresTextures], useLowresTexture),
    ], fileType == SWFType.SC1); 

    return SubMenu(
        Locale.Get("TID_TEXTURES_LABEL"),
        "texture_settings",
        {
            marginBottom: "6px"
        },
        ...versionDependentProps,
        ...generalProps,
        ...encodingDependentProps
    )
}