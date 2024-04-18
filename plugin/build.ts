import which from "which";
import { join } from "path";
import { copyDir, isMac, isWindows, makeLink, progress } from "../scripts/utils";
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

const [MAJOR, MINOR, MAINTENANCE] = version.split(".");

const defaultBuildDirectory = join(__dirname, "build");

const releaseBuildDirectory = join(__dirname, "build_release");

const buildDirectory = isDev ? defaultBuildDirectory : releaseBuildDirectory;

const ReleaseCmakeFlags = [
    "-DBUILD_SHARED_LIBS=OFF",
    "-DSC_ANIMATE_IMAGE_DEBUG=OFF",
    "-DBUILD_WITH_STATIC_CRT=OFF",
    `-DFETCHCONTENT_BASE_DIR=${join(defaultBuildDirectory, "_deps")}`
]
const DebugCmakeFlags = [
    "-DBUILD_SHARED_LIBS=ON",
    "-DSC_ANIMATE_IMAGE_DEBUG=ON",
]

const CmakeFlags = (isDev ? DebugCmakeFlags : ReleaseCmakeFlags).join(" ");

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

    const outputDirectory = join(libPath, "win");
    const binaryDirectory = join(buildDirectory, "bin", activeConfiguration);

    exec(`"${cmakePath}" -S "${__dirname}" -B "${buildDirectory}" ${CmakeFlags}`);

    if (isDev)
    {
        mkdirSync(binaryDirectory, { recursive: true });
        makeLink(binaryDirectory, outputDirectory);
        return;
    }

    const command = `"${cmakePath}" --build "${buildDirectory}" --config ${activeConfiguration} --target ScAnimatePlugin`
    progress(command)

    exec(command);

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
