import { CSEvent, getInterface, isCEP } from "../CEP"

enum PublisherModes {
  JSON  
}

interface ModuleState {
    output: string,
    publisher: string
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
            publisher: PublisherModes[PublisherModes.JSON]
        }
    }
}

export function getParam(name: keyof ModuleState): any {
    return State.PublishSettings.SupercellSWF[name];
}

export function setParam(name: keyof ModuleState, value: any) {
    State.PublishSettings.SupercellSWF[name]  = value;
    
}

export function restoreState(event: CSEvent) {
    const data: typeof State = JSON.parse(event.data);

    Object.assign(State.PublishSettings.SupercellSWF, data.PublishSettings.SupercellSWF);
}

export function saveState() {
  if (!isCEP()) {
    return;
  }
  const CSInterface = getInterface();

  var event = new CSEvent(
    "com.adobe.events.flash.extension.savestate",
    "APPLICATION"
  );
  event.data = JSON.stringify(State);
  event.extensionId = CSInterface.getExtensionID();
  CSInterface.dispatchEvent(event);
}