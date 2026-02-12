class SystemInfo {
	constructor() {
		const context = window.SupercellSWF;
		const [os, _version] = fl.version.split(" ");

		if (os == "WIN") {
			const info_output_path = `${window.SupercellSWF.cwd}user_data.txt`;
			const info_executable = `${context.binary_path}/info`;
			const command = `call "${FLfile.uriToPlatformPath(info_executable)}" "${FLfile.uriToPlatformPath(info_output_path)}"`;
			const status = FLfile.runCommandLine(command);
			if (!status) {
				fl.trace("Failed to get system info!");
			}

			const data = FLfile.read(info_output_path);
			const [cep_path, cpu_features] = data.match(/[^\r\n]+/g);

			this.install_path = cep_path.replace(/\\/g, "\\\\");
			this.cpu_features = cpu_features.split(",");
		} else {
			this.install_path = "$HOME/Library/Application Support/Adobe/CEP/"
		}
	}

	/**
	 * Path to CEP extensions (User scope)
	 */
	public install_path: string = "";

	/**
	 * Current system cpu supported instruction set
	 * Windows feature only
	 */
	public cpu_features: string[] = [];
}

class Localization {
	constructor() {
		let locale_path = `${window.SupercellSWF.cwd}core/locales/${this.lang}.json`;
		if (!FLfile.exists(locale_path)) {
			locale_path = `${window.SupercellSWF.cwd}core/locales/en_US.json`;
		}

		this.data = JSON.parse(FLfile.read(locale_path));
	}

	get(tid: string): string {
		if (this.data[tid] === undefined) {
			return tid;
		}

		return this.data[tid];
	}

	private lang = fl.languageCode;
	private data: object = {};
}

(() => {
	// @ts-expect-error
	window.SupercellSWF = {
		cwd: fl.scriptURI.replace("core/initialize_scope.jsfl", ""),
		manifest: undefined,
		locale: undefined,
		system: undefined,
		error_message: "",
	};

	// Initialize polyfills
	fl.runScript(`${window.SupercellSWF.cwd}core/polyfill/string.jsfl`);
	fl.runScript(`${window.SupercellSWF.cwd}core/polyfill/array.jsfl`);
	fl.runScript(`${window.SupercellSWF.cwd}core/polyfill/JSON.jsfl`);

	const [os, _version] = fl.version.split(" ");
	if (os !== "WIN" && os !== "MAC") {
		window.SupercellSWF.error_message = "Unsupported OS";
		return;
	}

	// Reading Manifest
	window.SupercellSWF.manifest_path = `${window.SupercellSWF.cwd}manifest.json`;
	window.SupercellSWF.manifest = JSON.parse(
		FLfile.read(window.SupercellSWF.manifest_path),
	) as any;
	window.SupercellSWF.user_manifest_uri = `${fl.configURI + window.SupercellSWF.manifest.name}.manifest.json`;
	window.SupercellSWF.user_manifest = FLfile.exists(
		window.SupercellSWF.user_manifest_uri,
	)
		? (JSON.parse(FLfile.read(window.SupercellSWF.user_manifest_uri)) as any)
		: undefined;

	// Localization Init
	window.SupercellSWF.locale = new Localization();

	window.SupercellSWF.binary_path = `${window.SupercellSWF.cwd}bin/`;
	window.SupercellSWF.os = os;

	window.SupercellSWF.system = () => {
		if (!window.SupercellSWF._system) {
			window.SupercellSWF._system = new SystemInfo();
		}

		return window.SupercellSWF._system;
	};
})();
