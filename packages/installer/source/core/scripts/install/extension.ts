function install(extension: Extension) {
	const context = window.SupercellSWF;
	const system = context.system();

	const package_path = context.cwd + extension.path;
	const unpack_log = `${window.SupercellSWF.cwd}unpack_log.txt`;

	const extensions_folder = `${FLfile.platformPathToURI(system.install_path)}extensions/`;
	const destination_folder = extensions_folder + extension.install;
	if (FLfile.exists(destination_folder)) {
		FLfile.remove(destination_folder);
	}

	FLfile.createFolder(destination_folder);

	let command;
	if (context.os == "WIN") {
		const archiver_bin = `${context.binary_path}7z.exe`;
		command = `call "${FLfile.uriToPlatformPath(archiver_bin)}" x -y "${FLfile.uriToPlatformPath(package_path)}" -o"${FLfile.uriToPlatformPath(destination_folder)}" > "${FLfile.uriToPlatformPath(unpack_log)}" 2>&1`;
	} else if (context.os == "MAC") {
		command = `ditto -x -k "${FLfile.uriToPlatformPath(package_path)}" "${FLfile.uriToPlatformPath(destination_folder)}" > "${FLfile.uriToPlatformPath(unpack_log)}" 2>&1`;
	}

	const status = FLfile.runCommandLine(command);
	if (!status) {
		fl.trace(`Failed to unpack ${extension.path}`);
	}
}

(() => {
	const context = window.SupercellSWF;
	const system = context.system();

	const installed = [];
	for (const baseExtension of context.manifest.extensions) {
		if (baseExtension.type !== "extension") continue;

		const extension = baseExtension as Extension;
		if (installed.indexOf(extension.name) != -1) continue;

		let valid = true;
		if (extension.condition) {
			const conditions = extension.condition.split(",");
			for (const condition of conditions) {
				const [prop, value] = condition.split(":");

				if (prop == "cpuf") {
					if (system.cpu_features.indexOf(value) == -1) {
						valid = false;
						break;
					}
				}
			}
		}
		if (!valid) continue;

		if (extension.variant_name) {
			if (context.os == "WIN") {
				fl.trace(
					`Installed "${extension.name}" extension with "${extension.variant_name}" variant`,
				);
			} else {
				fl.trace(
					`Installed "${extension.name}" extension with`,
				);
			}
		}

		install(extension);
		installed.push(extension.name);
	}
})();
