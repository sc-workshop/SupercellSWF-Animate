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
    None,
    Half,
    Quarter

}

export const TextureDimensions = [
    512,
    1024,
    2048,
    4096
]

interface PublisherStateData {
    //Basic settings
    output: string,
    method: PublisherMethods,

    // Additional settings
    compressionMethod: CompressionMethods,

    // Export to another file settings
    exportToExternal: boolean,
    exportToExternalPath: string,

    // Texture category
    hasExternalTexture: boolean,
    textureScaleFactor: TextureScaleFactor
    textureMaxWidth: number,
    textureMaxHeight: number

}

export class PublisherState {
    data: PublisherStateData = {
        output: "",
        method: PublisherMethods.SWF,

        compressionMethod: CompressionMethods.LZMA,

        exportToExternal: false,
        exportToExternalPath: "",

        // Textures
        hasExternalTexture: true,
        textureScaleFactor: TextureScaleFactor.None,
        textureMaxWidth: 2048,
        textureMaxHeight: 2048
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
