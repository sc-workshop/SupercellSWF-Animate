import { CSEvent, getInterface, isCEP } from "../CEP"

interface ModuleState {
    [key: string]: string
}

interface PublisherState {
    PublishSettings: {
        SupercellSWF: ModuleState
    }
}

export const State: PublisherState = {
    PublishSettings: {
        SupercellSWF: {
            output: "",
            debug: "1"
        }
    }

}

export function getParam(name: keyof ModuleState): any {
    return State.PublishSettings.SupercellSWF[name];
}

export function setParam(name: keyof ModuleState, value: any) {
    State.PublishSettings.SupercellSWF[name] = value;

}

export function restoreState(event: CSEvent) {
    const data: { [key: string]: any } = JSON.parse(event.data);

    function setKeys(key: string, value: any) {
        const obj: { [key: string]: any } = {};

        const keys = key.split('.');
        if (keys.length === 1) {
            obj[keys[0]] = value;
        } else if (keys.length > 1) {
            obj[keys[0]] = setKeys(
                keys.slice(1, keys.length).join('.'),
                value
            );
        }

        return obj
    };

    for (const key of Object.keys(data)) {
        if (Object.keys(State).includes(key)) {
            Object.assign(State[key as keyof PublisherState], setKeys(key, data[key]));
        }
    }

    Object.assign(State, data);
}

export function saveState() {
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
            data[key] = object;
        }
    }

    for (const key of Object.keys(State)) {
        setKeys(key, State[key as keyof PublisherState]);
    }

    event.data = JSON.stringify(data);
    event.appId = CSInterface.getApplicationID();
    event.extensionId = "com.scwmake.SupercellSWF.PublishSettings";
    CSInterface.dispatchEvent(event);
}