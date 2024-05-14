function string_trim(str: string)
{
    return str.replace(/^[\s\uFEFF\xA0]+|[\s\uFEFF\xA0]+$/g, "");
}

function install_windows(extension: Extension) {
    const package_path = FLfile.uriToPlatformPath(window.SupercellSWF.cwd + extension.path);
    const archiver_bin = FLfile.uriToPlatformPath(window.SupercellSWF.cwd + "core/bin/windows/7z.exe");
    const program_data_file = window.SupercellSWF.cwd + "program_data.txt";
    const unpack_log = window.SupercellSWF.cwd + "unpack_log.txt";

    FLfile.runCommandLine("echo %CommonProgramFiles% > \"" + FLfile.uriToPlatformPath(program_data_file) + "\"");
    const program_files_path = FLfile.platformPathToURI(string_trim(FLfile.read(program_data_file).split("\r\n")[0]));
    FLfile.remove(program_data_file);

    const extensions_folder = program_files_path + "/Adobe/CEP/extensions/";
    const destination_folder = extensions_folder + extension.install;

    if (FLfile.exists(destination_folder))
    {
        FLfile.remove(destination_folder);
    }

    FLfile.createFolder(destination_folder);
    FLfile.runCommandLine("call \"" + archiver_bin + "\" x \"" + package_path + "\" -o\"" + FLfile.uriToPlatformPath(destination_folder) + "\" >> \"" + FLfile.uriToPlatformPath(unpack_log) + "\"")
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