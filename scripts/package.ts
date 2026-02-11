import { execSync } from "node:child_process";
import {
	copyFileSync,
	existsSync,
	mkdirSync,
	unlinkSync,
	writeFileSync,
} from "node:fs";
import { basename, join, relative } from "node:path/posix";
import { cwd } from "node:process";
import { version } from "../package.json";
import cert from "./cert";
import {
	bundleId,
	config,
	extensionDistFolder,
	packageFolder,
} from "./manifest";
import { isWindows, log, removeDirs } from "./utils";
import { CommandExtension, BaseExtension, NativeExtension } from "./manifest/interfaces";

function build_extension_package(
	output_package: string,
	additional_args: string = "",
) {
	const rebuild_package =
		process.argv.indexOf("--raw-package") == -1 ||
		!existsSync(extensionDistFolder);

	if (rebuild_package) {
		if (existsSync(extensionDistFolder)) {
			removeDirs(extensionDistFolder);
		}
	}

	if (rebuild_package) {
		log(`Building production build for package...`);
		execSync(`pnpm run build:prod ${additional_args}`, { stdio: [0, 1, 2] });
	}

	const zxpCmd = isWindows ? `ZXPSignCmd.exe` : `./ZXPSignCmd`;
	const cwdDir = join(__dirname, "zxp", process.platform);
	const certPath = join(process.cwd(), "cert.p12");

	if (existsSync(certPath)) {
		unlinkSync(certPath);
	}

	execSync(
		`${zxpCmd} -selfSignedCert "${cert.countryCode}" "${cert.province}" "${cert.organization}" "${bundleId}" "${cert.password}" "${certPath}"`,
		{ cwd: cwdDir, stdio: [0, 1, 2] },
	);

	execSync(
		`${zxpCmd} -sign "${extensionDistFolder}" "${output_package}" "${certPath}" "${cert.password}"`,
		{ cwd: cwdDir, stdio: [0, 1, 2] },
	);

	log(`Package saved to ${output_package}`);

	removeDirs(join(extensionDistFolder, bundleId));
}

if (existsSync(packageFolder)) {
	removeDirs(packageFolder);
	mkdirSync(packageFolder, { recursive: true });
}

// Copying installer runtime
execSync(`pnpm build ${packageFolder}`, {
	stdio: [0, 1, 2],
	cwd: join(cwd(), "packages", "installer"),
});

const packageDistFolder = join(packageFolder, "packages");
const packageCommandsFolder = join(packageDistFolder, "commands");
mkdirSync(packageDistFolder, { recursive: true });
mkdirSync(packageCommandsFolder, { recursive: true });

// Building extension
interface ExtensionVariants {
	name: string;
	featureSet: string;
}

const defaultVariant = {
	name: "",
	featureSet: "Default"
}

const windowsVariants: ExtensionVariants[] = [
	defaultVariant,
	{
		name: "sse4.1",
		featureSet: "SSE41"
	},
	{
		name: "avx2",
		featureSet: "AVX2"
	}
]

const packageManifest: any = {
	name: bundleId,
	version: version,
	extensions: [],
};

function createCommand(name: string, extension: CommandExtension) {
	const scriptName = basename(extension.path);
	const scriptDist = join(packageCommandsFolder, scriptName);
	copyFileSync(extension.path, scriptDist);

	packageManifest.extensions.push({
		type: "command",
		name: name,
		path: relative(packageFolder, scriptDist),
		install: `${scripInstallName}/${scriptName}`,
	});
}

function createExtension(name: string, extension: BaseExtension) {
	const useFeatureSets = extension.type == "native" && (extension as NativeExtension).useFeatureSets;
	const targetVariants = isWindows && useFeatureSets ? windowsVariants : [defaultVariant];

	for (const variant of targetVariants) {
		const package_name = [bundleId, version, variant.name]
			.filter((value) => { return value != ""; })
			.join("-");

		const extensionPackagePath = join(packageDistFolder, `${package_name}.zxp`);
		build_extension_package(extensionPackagePath, `--fresh --cpuf=${variant.featureSet}`);

		let extensionManifest: any = {
			type: "extension",
			name: name,
			path: relative(packageFolder, extensionPackagePath),
			install: bundleId,
		}

		if (variant.name !== "") {
			extensionManifest.condition = `cpuf:${variant.featureSet}`;
			extensionManifest.variant_name = variant.name;
		}

		packageManifest.extensions.push(extensionManifest);
	}
}

const scripInstallName = config.organization_name
	? config.organization_name
	: config.organization;

for (const extensionName of Object.keys(config.extensions)) {
	const extension = config.extensions[extensionName];

	switch (extension.type) {
		case "extension": {
			createExtension(extensionName, extension);
			break;
		}
		case "command": {
			createCommand(extensionName, extension as CommandExtension);
			break;
		}
	}
}

writeFileSync(
	join(packageFolder, "manifest.json"),
	JSON.stringify(packageManifest),
);
