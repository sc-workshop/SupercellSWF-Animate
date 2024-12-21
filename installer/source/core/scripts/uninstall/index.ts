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
                    path = cep_path + "extensions/" + extension.install + "CSXS";
                    break;

                case "command":
                    path = fl.configURI + "Commands/" + extension.install;
                    break;
            }

            if (path)
            {
                FLfile.remove(path);
            }
        }

        FLfile.remove(window.SupercellSWF.user_manifest_path);
    }
)()