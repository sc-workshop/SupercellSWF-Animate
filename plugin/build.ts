import which from "which";
import { join } from "path";
import { copyDir, isMac, isWindows, log, makeLink, progress } from "../scripts/utils";
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

const isFresh = args.indexOf("--fresh") != -1;

let cpuFeature = "SSE2"
for (let arg of args)
{
    if (arg.startsWith("--cpuf"))
    {
        cpuFeature = arg.split("=")[1];
        break;
    }
}

const libPath = join(outputPath, "lib");
mkdirSync(libPath, { recursive: true });

const activeConfiguration = isDev ? "Debug" : "Release"

const publisherName = "SupercellSWF"
const publisherId = "org.scWorkshop.SupercellSWF.Publisher";
const publisherUi = "org.scWorkshop.SupercellSWF.PublishSettings"

const doctypeName = "SupercellSWF";
const doctypeId = "org.scWorkshop.SupercellSWF";

const assetsFolder = "resources"

const buildDirectory = join(__dirname, "build");
const outputDirectory = join(libPath, "win");
const binaryDirectory = join(buildDirectory, "animate_bin", activeConfiguration);

const [MAJOR, MINOR, MAINTENANCE] = version.split(".");

const CmakeFlagsList = [
    `-DBUILD_SHARED_LIBS=${isDev ? "ON" : "OFF"}`, // Build static lib for Release
    "-DBUILD_WITH_STATIC_CRT=OFF",
    `${isDev || !isFresh ? "" : "--fresh"}`, // build from fresh for each release build just to make sure that everything will be ok
    `-DWK_PREFERRED_CPU_FEATURES=${cpuFeature}`,
    '-DCMAKE_POLICY_VERSION_MINIMUM=3.5'
]

const CmakeFlags = CmakeFlagsList.join(" ");

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

    log(`Running cmake with flags: ${CmakeFlags}`)
    exec(`"${cmakePath}" -S "${__dirname}" -B "${buildDirectory}" ${CmakeFlags}`);
    exec(`"${cmakePath}" --build "${buildDirectory}" --config ${activeConfiguration} --target ScAnimatePlugin`);

    copyDir(binaryDirectory, outputDirectory);
    progress("Done");
}

function buildMac() {
    throw new Error();
}

const config =
    "#pragma once\n" +
    "\n" +
    "namespace sc::Adobe\n" +
    "{\n" +
    `constexpr const char*  PUBLISHER_NAME =						"${publisherName}";\n` +
    `constexpr const char*  PUBLISHER_UNIVERSAL_NAME =			    "${publisherId}";\n` +
    '\n' +
    `constexpr const char*  PUBLISH_SETTINGS_UI_ID =				"${publisherUi}";\n` +
    '\n' +
    `constexpr const char*  DOCTYPE_NAME =						    "${doctypeName}";\n` +
    `constexpr const char*  DOCTYPE_UNIVERSAL_NAME =				"${doctypeId}";\n` +
    '\n' +
    `constexpr int  PLUGIN_VERSION_MAJOR =				            ${MAJOR};\n` +
    `constexpr int  PLUGIN_VERSION_MINOR =				            ${MINOR};\n` +
    `constexpr int  PLUGIN_VERSION_MAINTENANCE =			        ${MAINTENANCE};\n` +
    `#define  __SC_ANIMATE_VERSION__			                    "${MAJOR}.${MINOR}.${MAINTENANCE}"\n` +
    "}"

writeFileSync(
    "supercell-flash-plugin/source/PluginConfiguration.h",
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
