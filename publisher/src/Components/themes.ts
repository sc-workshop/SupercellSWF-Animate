import { isCEP } from "../CEP";

export enum AppThemes {
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

export let AppTheme: AppThemes = AppThemes.Dark;

export let AppColor = AppTheme === AppThemes.Dark ? 0x333333 : 0xf1f1f;