
class Localization {
    constructor() {
        let locale_path = window.SupercellSWF.cwd + "core/locales/" + this.lang + ".json";
        if (!FLfile.exists(locale_path)) {
            locale_path = window.SupercellSWF.cwd + "core/locales/en_US.json";
        }

        this.data = JSON.parse(FLfile.read(locale_path));
    }

    get(tid: string): string {
        if (this.data[tid] === undefined) {
            return tid;
        }

        return this.data[tid];
    }

    private lang = fl.languageCode;
    private data: object = {};
}

function get_user_cep_windows(): string {
    const appdata_output_temp = window.SupercellSWF.cwd + "program_data.txt";
    let status = FLfile.runCommandLine("echo %appdata% > \"" + FLfile.uriToPlatformPath(appdata_output_temp) + "\"");
    if (status != 1) return "";

    const appdata_path = FLfile.platformPathToURI(FLfile.read(appdata_output_temp).split("\r\n")[0].trim());
    FLfile.remove(appdata_output_temp);

    return appdata_path + "/Adobe/CEP/"
}

(
    function () {
        // @ts-ignore
        window.SupercellSWF = {
            cwd: fl.scriptURI.replace("core/initialize_scope.jsfl", ""),
            manifest: undefined,
            locale: undefined,
            user_cep: undefined,
            error_message: ""
        }

        const [os, version] = fl.version.split(" ");
        if (os !== "WIN") {
            window.SupercellSWF.error_message = "Unsupported OS";
        }

        // Initialize polyfills
        fl.runScript(window.SupercellSWF.cwd + "core/polyfill/string.jsfl")
        fl.runScript(window.SupercellSWF.cwd + "core/polyfill/array.jsfl")
        fl.runScript(window.SupercellSWF.cwd + "core/polyfill/JSON.jsfl")

        // Reading Manifest 
        window.SupercellSWF.manifest_path = window.SupercellSWF.cwd + "manifest.json";
        window.SupercellSWF.manifest = JSON.parse(FLfile.read(window.SupercellSWF.manifest_path)) as any;
        window.SupercellSWF.user_manifest_path = fl.configURI + window.SupercellSWF.manifest.name + ".manifest.json";
        window.SupercellSWF.user_manifest =
            FLfile.exists(window.SupercellSWF.user_manifest_path) ?
                JSON.parse(FLfile.read(window.SupercellSWF.user_manifest_path)) as any : undefined;

        // Localization Init
        window.SupercellSWF.locale = new Localization();

        // CEP Variables Init
        window.SupercellSWF.user_cep = () => {
            if (!window.SupercellSWF._user_cep) {
                window.SupercellSWF._user_cep = os == "WIN" ? get_user_cep_windows() : undefined;
                if (!window.SupercellSWF._user_cep) {
                    window.SupercellSWF.error_message = "Failed to get path to CEP directory";
                }
            }

            return window.SupercellSWF._user_cep;
        }
    }
)()

