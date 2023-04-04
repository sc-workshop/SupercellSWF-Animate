
import { getInterface, isCEP } from "../CEP";
import { restoreState as setState, saveState } from "./publisherState";
import { refreshTheme } from "./themes";

export function publish() {
    if (!isCEP) {
        return;
    }

    saveState();
    const CSInterface = getInterface();
    CSInterface.evalScript("fl.getDocumentDOM().publish()", function() {
        CSInterface.closeExtension();
    })
}

export function onLoad() {
    if (isCEP()) {
        refreshTheme();
        const CSInterface = getInterface();
        CSInterface.addEventListener("com.adobe.events.flash.extension.setstate", setState);
    }
}