import { CSEvent, getInterface, isCEP } from "../CEP"

export enum PublisherMethods {
    JSON,
    SWF
}

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

enum ExportsMode {
    AllMovieclips,
    AllUnusedMovieclips
}

interface ExportName {
    path: string
}

interface PublisherStateData {
    //Basic settings
    output: string,
    method: PublisherMethods,

    // Additional settings
    compressionMethod: CompressionMethods,
    filledShapeOptimization: boolean,
    hasPrecisionMatrices: boolean,

    // Export to another file settings
    exportToExternal: boolean,
    exportToExternalPath: string,

    // Texture category
    hasExternalTexture: boolean,
    textureEncoding: TextureEncoding,
    textureQuality: Quality,
    textureScaleFactor: TextureScaleFactor
    textureMaxWidth: number,
    textureMaxHeight: number,

    exportsMode: ExportsMode,
    exports: ExportName[]

}

export class PublisherState {
    data: PublisherStateData = {
        output: "",
        method: PublisherMethods.SWF,

        compressionMethod: CompressionMethods.LZMA,
        filledShapeOptimization: true,
        hasPrecisionMatrices: false,

        exportToExternal: false,
        exportToExternalPath: "",

        // Textures
        hasExternalTexture: true,
        textureEncoding: TextureEncoding.Raw,
        textureQuality: Quality.highest,
        textureScaleFactor: TextureScaleFactor["x1.0"],
        textureMaxWidth: 2048,
        textureMaxHeight: 2048,

        exportsMode: ExportsMode.AllUnusedMovieclips,
        exports: []
    };

    getParam(name: keyof PublisherStateData): any {
        return this.data[name];
    }

    setParam(name: keyof PublisherStateData, value: any) {
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

export const State = new PublisherState();
