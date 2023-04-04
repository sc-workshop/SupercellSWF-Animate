import { getInterface, isCEP } from "../CEP";

export enum themes {
    Dark,
    Light
}

export let theme: themes = themes.Dark;

export function refreshTheme() {
    if (!isCEP()) {
        return;
    }
    const CSInterface = getInterface();
    alert(CSInterface.getHostEnvironment());
    //const skinInfo = JSON.parse().appSkinInfo;
}
