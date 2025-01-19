(
    function()
    {
        const system = window.SupercellSWF.system();
        const cep_uri = FLfile.platformPathToURI(system.install_path);
        let uninstalled: string[] = [];

        for (const extension of window.SupercellSWF.user_manifest.extensions)
        {
            if (uninstalled.indexOf(extension.name) != -1) continue;

            var path = "";
            switch (extension.type)
            {
                case "extension":
                    // Let's just delete manifest folder
                    // Yes, it will leave garbage but we can't do more in such an environment
                    path = cep_uri + "extensions/" + extension.install + "/CSXS";
                    break;

                case "command":
                    path = fl.configURI + "Commands/" + extension.install;
                    break;
            }
            
            function warn_failure()
            {
                fl.trace("Failed to remove \"" + extension.name + "\" by path \"" + path + "\"")
            }
            
            if (path)
            {
                if (!FLfile.exists(path))
                {
                    warn_failure();
                } else 
                {
                    let rem_success = FLfile.remove(path);
                    if (!rem_success)
                    {
                        warn_failure();
                    }
                }
            }

            uninstalled.push(extension.name);
        }

        FLfile.remove(window.SupercellSWF.user_manifest_uri);
    }
)()