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

interface ModuleStateInterface {
    output: string,
    method: PublisherMethods,

    compression: string,

    hasTexture: boolean,
    textureScaleFactor: TextureScaleFactor
    textureMaxWidth: number,
    textureMaxHeight: number
    
}

interface PublisherStateInterface {
    PublishSettings: {
        SupercellSWF: ModuleStateInterface
    }
}

export class PublisherState {
    data: PublisherStateInterface = {
        PublishSettings: {
            SupercellSWF: {
                output: "",
                method: PublisherMethods.SWF,
                
                compression: "LZMA",

                // Textures
                hasTexture: true,
                textureScaleFactor: TextureScaleFactor.None,
                textureMaxWidth: 2048,
                textureMaxHeight: 2048
            }
        },
    };

    getParam(name: keyof ModuleStateInterface): any {
        return this.data.PublishSettings.SupercellSWF[name];
    }

    setParam(name: keyof ModuleStateInterface, value: any) {
        this.data.PublishSettings.SupercellSWF[name] = value as never;
    }

    save() {
        const data: { [name: string]: any } = {};

        function setKeys(key: string, object: any) {
            if (object instanceof Object) {
                for (const objKey of Object.keys(object)) {
                    setKeys(`${key}.${objKey}`, object[objKey]);
                }
            } else {
                data[key] = object;

            }
        }

        for (const key of Object.keys(this.data)) {
            setKeys(key, this.data[key as keyof PublisherStateInterface]);
        }

        console.log(data);

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

        event.data = JSON.stringify(data);
        CSInterface.dispatchEvent(event);
    }

    restore(data: any) {
        function setKeys(obj: any, key: string, value: any) {
            const keys = key.split('.');
            if (keys.length === 1) {
                obj[keys[0]] = value;
                
            } else if (keys.length > 1) {
                setKeys(
                    obj[keys[0]], keys.slice(1, keys.length).join('.'),
                    value
                );
            }
        };
    
        for (const key of Object.keys(data)) {
            setKeys(this.data, key, data[key as any]);
        }
    }
}

export const State = new PublisherState();
