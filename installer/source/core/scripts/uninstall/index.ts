(
    function()
    {
        const cep_path = window.SupercellSWF.user_cep();

        for (const extension of window.SupercellSWF.user_manifest.extensions)
        {
            var path = "";
            switch (extension.type)
            {
                case "extension":
                    // Let's just delete manifest folder
                    // Yes, it will leave garbage but we can't do more in such an environment
                    path = cep_path + "extensions/" + extension.install + "/CSXS";
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
                    continue;
                }

                let rem_success = FLfile.remove(path);
                if (!rem_success)
                {
                    warn_failure();
                }
            }
        }

        FLfile.remove(window.SupercellSWF.user_manifest_path);
    }
)()