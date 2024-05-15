
class Localization {
    constructor(cwd: string) {
        let locale_path = cwd + "core/locales/" + this.lang + ".json";
        if (!FLfile.exists(locale_path)) {
            locale_path = cwd + "core/locales/en_US.json";
        }

        this.data = JSON.parse(FLfile.read(locale_path));
    }

    get(tid: string) {
        if (this.data[tid] === undefined) {
            return tid;
        }

        return this.data[tid];
    }

    public lang = fl.languageCode;
    public data: object = {};
}

(
    function () {
        let current_path = fl.scriptURI.replace("core/initialize_scope.jsfl", "");

        // Initialize polyfills
        fl.runScript(current_path + "core/polyfill/string.jsfl")
        fl.runScript(current_path + "core/polyfill/JSON.jsfl")
        
        // Initializing global object
        const manifest_path = current_path + "manifest.json";
        const global_object =
        {
            cwd: current_path,
            manifest: JSON.parse(FLfile.read(manifest_path)) as any,
            locale: new Localization(current_path)
        }
        window.SupercellSWF = global_object
    }
)()

