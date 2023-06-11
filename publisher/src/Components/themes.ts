import { getInterface, isCEP } from "../CEP";

export enum themes {
    Dark,
    Light
}

export function refreshTheme() {
    if (!isCEP()) {
        return;
    }
    //const CSInterface = getInterface();
    //const hostEnv = CSInterface.getHostEnvironment();
}

export let theme: themes = themes.Dark;

export let themeColor = theme === themes.Dark ? 0x333333 : 0xf1f1f;