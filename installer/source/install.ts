(
    function () {
        let current_path = fl.scriptURI.replace("install.jsfl", "");

        let dialog_path = current_path + "core/ui/install/installer.xul"
        fl.xmlPanel(dialog_path)
    }
)()