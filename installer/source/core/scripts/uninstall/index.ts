(
    function()
    {
        const cep_path = window.SupercellSWF.user_cep();

        for (const extension of window.SupercellSWF.user_manifest.extensions)
        {
            switch (extension.type)
            {
                case "extension":
                    FLfile.remove(cep_path + "extensions/" + extension.install);
                    break;

                case "command":
                    FLfile.remove(fl.configURI + "Commands/" + extension.install);
                    break;
            }
        }

        FLfile.remove(window.SupercellSWF.user_manifest_path);
    }
)()