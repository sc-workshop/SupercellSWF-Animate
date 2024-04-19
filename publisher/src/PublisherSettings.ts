import { CSEvent, getInterface, isCEP } from "./CEP"

export enum CompressionMethods {
    LZMA,
    LZHAM,
    ZSTD
}

export enum TextureScaleFactor {
    "x1.0",
    "x0.5",
    "x0.25"

}

export const TextureDimensions = [
    512,
    1024,
    2048,
    4096
]

export enum TextureEncoding {
    Raw,
    KTX
}

export enum Quality {
    highest,
    high,
    medium,
    low
} 

interface PublisherSettingsData {
    //Basic settings
    output: string,

    // Additional settings
    compressionMethod: CompressionMethods,
    hasPrecisionMatrices: boolean,
    //useSpritesForNineSlice: boolean,

    // Export to another file settings
    exportToExternal: boolean,
    exportToExternalPath: string,

    // Texture category
    hasExternalTexture: boolean,
    hasExternalCompressedTexture: boolean,
    hasLowresTexture: boolean,
    hasMultiresTexture: boolean,
    multiResolutinSuffix: string,
    lowResolutionSuffix: string,
    textureEncoding: TextureEncoding,
    textureQuality: Quality,
    textureScaleFactor: TextureScaleFactor
    textureMaxWidth: number,
    textureMaxHeight: number,
}

export class PublisherSettings {
    data: PublisherSettingsData = {
        output: "",

        compressionMethod: CompressionMethods.ZSTD,
        hasPrecisionMatrices: false,
        //useSpritesForNineSlice: false,

        exportToExternal: false,
        exportToExternalPath: "",

        // Textures
        hasExternalTexture: true,
        hasExternalCompressedTexture: true,
        hasLowresTexture: false,
        hasMultiresTexture: false,
        multiResolutinSuffix: "_highres",
        lowResolutionSuffix: "_lowres",
        textureEncoding: TextureEncoding.KTX,
        textureQuality: Quality.highest,
        textureScaleFactor: TextureScaleFactor["x1.0"],
        textureMaxWidth: 2048,
        textureMaxHeight: 2048,
    };

    getParam(name: keyof PublisherSettingsData): any {
        return this.data[name];
    }

    setParam(name: keyof PublisherSettingsData, value: any) {
        this.data[name] = value as never;
    }

    save() {
        if (!isCEP()) {
            return;
        }
        const CSInterface = getInterface();

        const event = new CSEvent(
            "com.adobe.events.flash.extension.savestate",
            "APPLICATION",
            CSInterface.getApplicationID(),
            "com.scwmake.SupercellSWF.PublishSettings"
        );

        event.data = JSON.stringify({
            SupercellSWF: JSON.stringify(this.data)
        });
        CSInterface.dispatchEvent(event);
    }
    
    restore(data: any) {
        console.log(data)
        try {
            if (data.SupercellSWF) {
                Object.assign(this.data, JSON.parse(data.SupercellSWF));
            }
        } catch (error) {
            alert("Failed to load publisher settings");
        }

    }
}

export const Settings = new PublisherSettings();
