import which from "which";
import { join } from "path";
import { copyDir, isMac, isWindows, makeLink, processExecError, progress } from "../scripts/utils";
import { execSync } from "child_process";
import { mkdirSync, writeFileSync, existsSync } from "fs";
import { version } from "./package.json"

if (!isWindows && !isMac) {
    throw new Error("Unsupported platform");
}

const args = process.argv;
const isDev = args[2] == "development";
const outputPath = args[3];
mkdirSync(outputPath, { recursive: true });

const libPath = join(outputPath, "lib");
mkdirSync(libPath, { recursive: true });

const activeConfiguration = isDev ? "Debug" : "RelWithDebInfo"

const publisherName = "SupercellSWF"
const publisherId = "com.scwmake.SupercellSWF.Publisher";
const publisherUi = "com.scwmake.SupercellSWF.PublishSettings"

const doctypeName = "SupercellSWF";
const doctypeId = "com.scwmake.SupercellSWF";

const assetsFolder = "resources"

const buildDirectory = join(__dirname, isDev ? "build" : "build_release");
const outputDirectory = join(libPath, "win");
const binaryDirectory = join(buildDirectory, "animate_bin", activeConfiguration);

const cmakeFlags = [
    "-DBUILD_SHARED_LIBS=OFF",
    "-DSC_ANIMATE_IMAGE_DEBUG=OFF",
    "-DBUILD_WITH_STATIC_CRT=OFF",
    "-DRP_BUILD_SHARED_LIBS=OFF",
    "-DLIBNEST2D_HEADER_ONLY=OFF"
]

const [MAJOR, MINOR, MAINTENANCE] = version.split(".");

function buildWindows() {
    const cmakePath = which.sync("cmake");
    if (cmakePath.length <= 0) {
        throw new Error("Failed to find CMake executable");
    }

    function exec(command: string)
    {
        execSync(command,
        { stdio: [0, 1, 2], cwd: __dirname 
    });
    }

    if (isDev)
    {
        mkdirSync(binaryDirectory, { recursive: true });
        makeLink(binaryDirectory, outputDirectory);
        progress("Now you can compile Binaries from IDE")
        return;
    }

    exec(`"${cmakePath}" -S "${__dirname}" -B "${buildDirectory}" ${cmakeFlags.join(" ")}"`);
    exec(`"${cmakePath}" --build "${buildDirectory}" --config ${activeConfiguration}`);

    copyDir(binaryDirectory, outputDirectory);
    progress("Done");
}

function buildMac() {
    throw new Error();
}

const config =
    "#pragma once\n" +
    "\n" +
    `#define PUBLISHER_NAME						"${publisherName}"\n` +
    `#define PUBLISHER_UNIVERSAL_NAME			"${publisherId}"\n` +
    '\n' +
    `#define PUBLISH_SETTINGS_UI_ID				"${publisherUi}"\n` +
    '\n' +
    `#define DOCTYPE_NAME						"${doctypeName}"\n` +
    `#define DOCTYPE_UNIVERSAL_NAME				"${doctypeId}"\n` +
    '\n' +
    `#define PLUGIN_VERSION_MAJOR				${MAJOR}\n` +
    `#define PLUGIN_VERSION_MINOR				${MINOR}\n` +
    `#define PLUGIN_VERSION_MAINTENANCE			${MAINTENANCE}\n`

writeFileSync(
    "include/PluginConfiguration.h",
    config,
    "utf-8"
)

const dstResourceFolder = join(libPath, assetsFolder);
if (!existsSync(dstResourceFolder)) {
    if (isDev) {
        makeLink(join(__dirname, assetsFolder), dstResourceFolder);
    } else {
        copyDir(assetsFolder, dstResourceFolder)
    }
}

if (isWindows) {
    buildWindows();
} else {
    buildMac();
}

writeFileSync(join(outputPath, "fcm.xml"), "");
