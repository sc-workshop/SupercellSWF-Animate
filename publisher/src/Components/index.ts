import { CSInterface } from "./CEP";
import { restoreState as setState, saveState, State } from "./publisherState";
import { refreshTheme } from "./themes";

export function publish() {
    alert(State.PublishSettings.SupercellSWF.output);

    if (!CSInterface) {
        return;
    }

    saveState();
}

export function onLoad() {
    if (CSInterface) {
        refreshTheme();
        CSInterface.addEventListener(CSInterface.THEME_COLOR_CHANGED_EVENT, refreshTheme);
        CSInterface.addEventListener("com.adobe.events.flash.extension.setstate", setState);
    }
}