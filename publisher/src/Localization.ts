import { getInterface, isCEP } from "./CEP"

enum Locales {
    en_EN = "LilitaOne.ttf",
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

        this.locale = require(`./locales/${Object.keys(Locales)[Object.values(Locales).indexOf(this.code)]}.json`);
    }

    locale: { [TID: string]: any } = {};

    code: Locales = Locales.en_EN;

    Get(TID: string): any {
        const value = this.locale[TID];
        if (value === undefined) {
            return TID;
        } else {
            return value;
        }
    }
}

const Locale = new LocaleInterface();
export default Locale