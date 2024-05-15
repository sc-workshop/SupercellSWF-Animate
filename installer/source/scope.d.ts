export { }

declare global {
    interface Extension {
        type: "command" | "extension",
        name: string,
        path: string,
        install: string
    }

    interface InstallManifest {
        name: string,
        version: string,
        extensions: Extension[]
    }

    interface ManifestWindow extends Window {
        SupercellSWF?: {
            cwd: string,

            // Package manifest
            manifest_path: string,
            manifest: InstallManifest,
            
            // User manifest. Undefined when user already has installed extension
            user_manifest_path: string,
            user_manifest?: InstallManifest

            locale: Localization,
            error_message: string,
            
            // Path to CEP folder with user extensions
            user_cep: () => string,
            // cached variable
            _user_cep?: string,
        }
    }

    var window: ManifestWindow;

    // Polyfills. Some ECMAScript functions not available in Animate 20-21 so just fill with our own functions
    interface String {
        trim(): string;
    }

    
    interface Array<T>
    {
        indexOf(element: T, fromIndex?: number) : number | -1;
    }
}

declare class Localization
{
    get(tid: string): string;
}

declare interface JSONClass {
    parse(data: string): any;
}

declare const JSON: JSONClass;