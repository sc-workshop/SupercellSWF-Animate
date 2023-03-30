import { CSInterface } from "./CEP";

export enum themes {
    Dark,
    Light
}

export let theme: themes = themes.Dark;

export function refreshTheme() {
    if (!CSInterface) {
        return;
    }
    alert(CSInterface.getHostEnvironment());
    //const skinInfo = JSON.parse().appSkinInfo;
}
