import { getInterface, isCEP } from "./CEP"
const defaultLocaleCode = 'en_EN';

class LocaleInterface {
    constructor() {
        let localeCode = defaultLocaleCode;

        if (isCEP()) {
            const cep = getInterface()
            const hostInfo = cep.getHostEnvironment();
            localeCode = hostInfo.appLocale;
        }

        try {
            this.locale = require(`../../locales/${localeCode}.json`);
        } catch(_) {
            console.error(`Failed to get locale for "${localeCode}"`);
            this.locale = require(`../../locales/${defaultLocaleCode}.json`)
        }
    }

    locale: {[TID: string]: any} = {};

    Get(TID: string): any {
        const value = this.locale[TID];
        if (value === undefined) {
            return TID;
        } else {
            return value;
        }
    }
}

export const Locale = new LocaleInterface();