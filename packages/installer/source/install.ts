(() => {
	const current_path = fl.scriptURI.replace("install.jsfl", "");

	const dialog_path = `${current_path}core/ui/install/installer.xul`;
	fl.xmlPanel(dialog_path);
})();
