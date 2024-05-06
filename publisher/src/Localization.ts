import { getInterface, isCEP } from "./CEP"

export enum Locales {
    en_US = "LilitaOne.ttf",
    ru_RU = "Pusia-Bold.otf"
}

function isSupportedLocale(value: string): value is Locales {
    return Object.keys(Locales).includes(value);
}

class LocaleInterface {
    constructor() {
        if (isCEP()) {
            const cep = getInterface()
            const hostInfo = cep.getHostEnvironment();
            const hostLocale = hostInfo.appLocale as Locales;

            if (isSupportedLocale(hostLocale)) {
                this.code = Object.values(Locales)[Object.keys(Locales).indexOf(hostLocale)];
            }
        }

        this.Load();
    }

    locale: { [TID: string]: unknown } = {};

    code: Locales = Locales.en_US;

    Load()
    {
        this.locale = require(`./locales/${Object.keys(Locales)[Object.values(Locales).indexOf(this.code)]}.json`)
    }

    Get(TID: string): string {
        const value = this.locale[TID];
        if (value === undefined) {
            return TID;
        } else {
            return String(value);
        }
    }
}

const Locale = new LocaleInterface();
export default Locale