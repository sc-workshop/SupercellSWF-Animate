export { }

declare global {
    interface Extension {
        type: "command" | "extension",
        name: string,
        path: string,
        install: string
    }

    interface InstallManifest {
        version: string,
        extensions: Extension[]
    }

    interface ManifestWindow extends Window {
        SupercellSWF?: {
            cwd: string,
            manifest: InstallManifest,
            locale: Localization,
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