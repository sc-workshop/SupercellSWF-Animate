import which from "which";
import { join } from "path";
import { progress } from "../scripts/utils";
import { execSync } from "child_process";
import { mkdirSync, copyFileSync, writeFileSync } from "fs";
import { version, description } from "./package.json"

const args = process.argv;
const isDev = args[2] == "development";
const outputPath = args[3];

const winSolution = "project/win/Plugin.sln";
const winDll = "build/win/Plugin.fcm";

const publisherName = "SupercellSWF"
const publisherId = "com.scwmake.ScSwfAnimate.PublishSettings";

const doctypeName = "SupercellSWF";
const doctypeId = "com.scwmake.ScSwfAnimate";

const [MAJOR, MINOR, MAINTENANCE] = version.split(".");

function buildWindows() {
    const msBuildPath = which.sync("msbuild");
    const solutionPath = join(__dirname, winSolution);

    progress("Building with MSBuild...");

    execSync(`"${msBuildPath}" "${solutionPath}" -property:Configuration=${isDev ? "Debug" : "Release"}`)

    progress("Done");

    const fcmOutputFolder = join(outputPath, "win");
    mkdirSync(fcmOutputFolder, { recursive: true });

    copyFileSync(winDll, join(fcmOutputFolder, "plugin.fcm"));
}

const config =
    "#pragma once\n" +
    "\n" +
    `#define PUBLISHER_NAME						"${publisherName}"\n` +
    `#define PUBLISHER_UNIVERSAL_NAME			"${publisherId}"\n` +
    '\n' +
    `#define PUBLISH_SETTINGS_UI_ID				"${publisherId}"\n` +
    '\n' +
    `#define DOCTYPE_NAME						"${doctypeName}"\n` +
    `#define DOCTYPE_UNIVERSAL_NAME				"${doctypeId}"\n` +
    `#define DOCTYPE_DESCRIPTION				"${description}"\n` +
    '\n' +
    `#define PLUGIN_VERSION_MAJOR				${MAJOR}\n` +
    `#define PLUGIN_VERSION_MINOR				${MINOR}\n` +
    `#define PLUGIN_VERSION_MAINTENANCE			${MAINTENANCE}\n`

writeFileSync(
    "include/PluginConfiguration.h",
    config,
    "utf-8"
)

switch (process.platform) {
    case "win32":
        buildWindows();
        break;
    case "darwin":
    default:
        throw Error("Unsupported platform");
}

writeFileSync(join(outputPath, "fcm.xml"), "");
