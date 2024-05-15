function install_windows(extension: Extension) {
    const package_path = FLfile.uriToPlatformPath(window.SupercellSWF.cwd + extension.path);
    const archiver_bin = FLfile.uriToPlatformPath(window.SupercellSWF.cwd + "core/bin/windows/7z.exe");
    const unpack_log = window.SupercellSWF.cwd + "unpack_log.txt";

    const extensions_folder = window.SupercellSWF.user_cep() + "extensions/";
    const destination_folder = extensions_folder + extension.install;

    if (FLfile.exists(destination_folder))
    {
        FLfile.remove(destination_folder);
    }

    FLfile.createFolder(destination_folder);
    FLfile.runCommandLine("call \"" + archiver_bin + "\" x -y \"" + package_path + "\" -o\"" + FLfile.uriToPlatformPath(destination_folder) + "\" >> \"" + FLfile.uriToPlatformPath(unpack_log) + "\"")
}

(
    function () {
        const [os, version] = fl.version.split(" ");
        for (const extension of window.SupercellSWF.manifest.extensions) {
            if (extension.type !== "extension") continue;

            switch (os) {
                case "WIN":
                    install_windows(extension);
                    break;

                default:
                    throw Error("Unknown platform")
            }
        }
    }
)()