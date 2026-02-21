import { execSync } from "node:child_process";
import { existsSync, mkdirSync, writeFileSync } from "node:fs";
import { join } from "node:path";
import which from "which";
import { getCmakePath } from "../../scripts/tools";
import {
	copyDir,
	isMac,
	isWindows,
	log,
	makeLink,
	progress,
} from "../../scripts/utils";
import { version } from "./package.json";

if (!isWindows && !isMac) {
	throw new Error("Unsupported platform");
}

const args = process.argv;
const isDev = args[2] == "development";
const outputPath = args[3];
mkdirSync(outputPath, { recursive: true });

const isFresh = args.indexOf("--fresh") != -1;

let cpuFeature = "SSE2";
for (const arg of args) {
	if (arg.startsWith("--cpuf")) {
		cpuFeature = arg.split("=")[1];
		break;
	}
}

const libPath = join(outputPath, "lib");
mkdirSync(libPath, { recursive: true });

const activeConfiguration = isDev ? "Debug" : "Release";

const publisherName = "SupercellSWF";
const publisherId = "org.scWorkshop.SupercellSWF.Publisher";
const publisherUi = "org.scWorkshop.SupercellSWF.PublishSettings";

const doctypeName = "SupercellSWF";
const doctypeId = "org.scWorkshop.SupercellSWF";

const assetsFolder = "resources";

const buildDirectory = join(__dirname, isDev ? "build" : "build-release");
const outputDirectory = join(libPath, isWindows ? "win" : "mac");
const binaryDirName = "animate_bin";
const binaryDirectory = join(
	buildDirectory,
	binaryDirName,
	activeConfiguration,
);

const [MAJOR, MINOR, MAINTENANCE] = version.split(".");

const MacOSFlags = ["-G Xcode"];

const WindowsFlags = [
	`-DVCPKG_FEATURE_FLAGS="${[cpuFeature == "avx2" ? "simd" : undefined].filter((value) => value !== undefined).join(";")}"`,
	`--preset Windows-${isDev ? "Debug" : "Release"}`,
	`-DWK_PREFERRED_CPU_FEATURES=${cpuFeature}`,
];

const CmakeFlagsList = [
	`-DBUILD_SHARED_LIBS=${isDev ? "ON" : "OFF"}`, // Build static lib for Release
	`${isDev || !isFresh ? "" : "--fresh"}`, // build from fresh for each release build just to make sure that everything will be ok
	`-DCMAKE_POLICY_VERSION_MINIMUM=3.5`,
	isMac ? MacOSFlags : undefined,
	isWindows ? WindowsFlags : undefined,
];

const CmakeFlags = CmakeFlagsList.flat()
	.filter((value) => {
		return value !== undefined;
	})
	.join(" ");

function exec(command: string) {
	execSync(command, { stdio: [0, 1, 2], cwd: __dirname });
}

function build() {
	const cmakePath = getCmakePath();

	// Configure project
	log(`Running cmake with flags: ${CmakeFlags}`);
	exec(`"${cmakePath}" -S "${__dirname}" -B "${buildDirectory}" ${CmakeFlags}`);

	if (isDev) {
		// Create directory with build artifacts ahead of time
		mkdirSync(binaryDirectory, { recursive: true });

		// Create link from artifacts folder to extension lib folder
		makeLink(binaryDirectory, outputDirectory);

		// Create link to resources folder
		makeLink(
			join(__dirname, "resources"),
			join(buildDirectory, binaryDirName, "resources"),
		);

		progress("Now you can compile Binaries from IDE");
		return;
	}

	// Execute build
	exec(
		`"${cmakePath}" --build "${buildDirectory}" --config ${activeConfiguration} --target ScAnimatePlugin --parallel`,
	);

	// Copy builded artifacts
	copyDir(binaryDirectory, outputDirectory);
	progress("Done");
}

const config =
	"#pragma once\n" +
	"\n" +
	"namespace sc::Adobe\n" +
	"{\n" +
	`constexpr const char*  PUBLISHER_NAME =						"${publisherName}";\n` +
	`constexpr const char*  PUBLISHER_UNIVERSAL_NAME =			    "${publisherId}";\n` +
	"\n" +
	`constexpr const char*  PUBLISH_SETTINGS_UI_ID =				"${publisherUi}";\n` +
	"\n" +
	`constexpr const char*  DOCTYPE_NAME =						    "${doctypeName}";\n` +
	`constexpr const char*  DOCTYPE_UNIVERSAL_NAME =				"${doctypeId}";\n` +
	"\n" +
	`constexpr int  PLUGIN_VERSION_MAJOR =				            ${MAJOR};\n` +
	`constexpr int  PLUGIN_VERSION_MINOR =				            ${MINOR};\n` +
	`constexpr int  PLUGIN_VERSION_MAINTENANCE =			        ${MAINTENANCE};\n` +
	`#define  __SC_ANIMATE_VERSION__			                    "${MAJOR}.${MINOR}.${MAINTENANCE}"\n` +
	"}";

writeFileSync(
	"supercell-flash-plugin/source/PluginConfiguration.h",
	config,
	"utf-8",
);

const dstResourceFolder = join(libPath, assetsFolder);
if (!existsSync(dstResourceFolder)) {
	if (isDev) {
		makeLink(join(__dirname, assetsFolder), dstResourceFolder);
	} else {
		copyDir(assetsFolder, dstResourceFolder);
	}
}

build();
writeFileSync(join(outputPath, "fcm.xml"), "");
