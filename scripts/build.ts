import { build_extension } from "./build/extension";
import { deploy } from "./deploy";
import { config, generateCSXS } from "./manifest";
import { isDev, progress } from "./utils";

const args = process.argv.slice(3);
const extensionsToBuild: string[] = [];
const flags: string[] = [];
for (const arg of args) {
	if (arg.startsWith("--")) {
		flags.push(arg);
	} else {
		extensionsToBuild.push(arg);
	}
}

progress(`BUILD for ${isDev ? "Development" : "Production"}`, "blue");

progress("Generating manifest..");
generateCSXS(config);

for (const extensionName of Object.keys(config.extensions)) {
	if (
		extensionsToBuild.length != 0 &&
		extensionsToBuild.indexOf(extensionName) === -1
	) {
		continue;
	}

	const extension = config.extensions[extensionName];
	progress(`Building ${extensionName}..`);

	switch (extension.type) {
		case "native":
		case "extension":
			build_extension(extensionName, extension, flags);
			break;
		default:
			break;
	}
}

if (isDev) {
	deploy();
}
