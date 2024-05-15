
function baseName(str: string)
{
   var base = new String(str).substring(str.lastIndexOf('/') + 1); 
    if(base.lastIndexOf(".") != -1)       
        base = base.substring(0, base.lastIndexOf("."));
   return base;
}

(
    function () {
        const default_commands_locale: string[] =
            [
                "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
                "<Commands>",
                "</Commands>"
            ];

        const commands_folder = fl.configURI + "Commands/";
        const commands_locale_path = commands_folder + "names.xml";
        const names_xml = FLfile.exists(commands_locale_path) ? FLfile.read(commands_locale_path).split("\n") : default_commands_locale;

        for (const extension of window.SupercellSWF.manifest.extensions) {
            if (extension.type !== "command") continue;

            const script_path = window.SupercellSWF.cwd + extension.path;
            const script_destination = commands_folder + extension.install;
            const script_destination_folder = script_destination.match(/(.*)[\/\\]/)[1] || '';
            FLfile.createFolder(script_destination_folder);
            FLfile.copy(script_path, script_destination);

            const script_tid = "TID_COMMAND_" + extension.name + "_NAME";
            const script_localized_name = window.SupercellSWF.locale.get(script_tid);
            if (script_tid != script_localized_name) {
                const xml_element = "  <name source=\"" + baseName(extension.path) + "\" target=\"" + script_localized_name + "\"></name>";
                if (names_xml.indexOf(xml_element) == -1)
                {
                    names_xml.splice(names_xml.length - 1, 0, xml_element)
                }
            }
        }

        FLfile.write(
            commands_locale_path,
            names_xml.join("\n")
        );
    }
)()