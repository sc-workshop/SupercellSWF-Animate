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
            manifest: InstallManifest
        }
    }

    var window: ManifestWindow;
}