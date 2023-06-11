import { CSEvent, getInterface, isCEP } from "../CEP"

interface ModuleStateInterface {
    output: string,
    debug: boolean,
    hasTexture: boolean,
    compression: string
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
                debug: false,
                hasTexture: true,
                compression: "LZMA"
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
        if (!isCEP()) {
            return;
        }
        const CSInterface = getInterface();

        const event = new CSEvent(
            "com.adobe.events.flash.extension.savestate",
            "APPLICATION"
        );

        const data: { [name: string]: any } = {};

        function setKeys(key: string, object: any) {
            if (object instanceof Object) {
                for (const objKey of Object.keys(object)) {
                    setKeys(`${key}.${objKey}`, object[objKey]);
                }
            } else {
                switch (typeof object) {
                    case "string":
                        data[key] = object;
                        break;

                    case "boolean":
                        data["key"] = object ? "1" : "0"
                        break;

                    case "undefined":
                        break;

                    default:
                        console.log(key, object)
                        throw new Error("unknown type")
                }

            }
        }

        for (const key of Object.keys(this.data)) {
            setKeys(key, this.data[key as keyof PublisherStateInterface]);
        }

        event.data = JSON.stringify(data);
        event.appId = CSInterface.getApplicationID();
        event.extensionId = "com.scwmake.SupercellSWF.PublishSettings";
        CSInterface.dispatchEvent(event);
    }

    restore(data: any) {
        function setKeys(obj: any, key: string, value: any) {
            const keys = key.split('.');
            if (keys.length === 1) {
                switch (typeof obj[keys[0]]) {
                    case "string":
                        obj[keys[0]] = value;
                        break;
                    case "boolean":
                        obj[keys[0]] = value === "1";
                        break;
                    case "undefined":
                        break;
                    default:
                        console.log(keys[0], value)
                        throw new Error("unknown data type");
                }
                
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
