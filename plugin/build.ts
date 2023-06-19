import which from "which";
import { join, basename } from "path";
import { copyDir, isMac, isWindows, makeLink, processExecError, progress } from "../scripts/utils";
import { execSync } from "child_process";
import { mkdirSync, copyFileSync, writeFileSync, existsSync } from "fs";
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

const winSolution = "ScAnimate.sln";

const publisherName = "SupercellSWF"
const publisherId = "com.scwmake.SupercellSWF.Publisher";
const publisherUi = "com.scwmake.SupercellSWF.PublishSettings"

const doctypeName = "SupercellSWF";
const doctypeId = "com.scwmake.SupercellSWF";

const [MAJOR, MINOR, MAINTENANCE] = version.split(".");

function buildWindows() {
    const sharedLibs = !isDev ?
        [
            "ThirdParty/AtlasGenerator/ThirdParty/lib/opencv/x86_64/windows/shared/opencv_world470.dll"
        ]
        :
        [
            "ThirdParty/AtlasGenerator/ThirdParty/lib/opencv/x86_64/windows/shared/opencv_world470d.dll"
        ]

    const msBuildPath = which.sync("msbuild");
    if (msBuildPath.length <= 0) {
        throw new Error("Failed to find MSBuild executable");
    }

    const solutionPath = join(__dirname, winSolution);

    if (!existsSync(winSolution)) {
        const premakePath = which.sync("premake5");
        if (premakePath.length <= 0) {
            throw new Error("Failed to find premake5 executable");
        }

        execSync(`scripts/generate.bat`, {stdio: [0, 1, 2]});
    }

    progress("Building with MSBuild...");

    if (!existsSync(solutionPath)) {
        throw new Error("Failed to get solution");
    }

    try {
        execSync(`"${msBuildPath}" "${solutionPath}" -property:Configuration=${isDev ? "Debug" : "Release"}`, {stdio: [0, 1, 2]})
    } catch (err) {
        throw processExecError(err);
    }

    progress("Done");

    if (!isDev) {
        const binaryPath = "bin/win/Plugin.fcm";
        if (!binaryPath) {
            throw new Error("Failed to get binary");
        }

        const fcmOutputFolder = join(libPath, "win");
        mkdirSync(fcmOutputFolder, { recursive: true });
        copyFileSync(binaryPath, join(fcmOutputFolder, "plugin.fcm"));

        for (const filepath of sharedLibs) {
            copyFileSync(join(__dirname, filepath), join(fcmOutputFolder, basename(filepath)))
        }
    }

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

const dstResourceFolder = join(libPath, "res");
if (isDev) {
    makeLink(join(__dirname, "res"), dstResourceFolder);
} else {
    copyDir("res", dstResourceFolder)
}

if (isWindows) {
    buildWindows();
} else {
    buildMac();
}

writeFileSync(join(outputPath, "fcm.xml"), "");
