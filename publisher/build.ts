import { copyDir, makeLink, progress } from "../scripts/utils";
import { join, resolve } from "path";

const args = process.argv;
const buildFolder = join(__dirname, "build");

const isDebug = args[2] === "development";
const outputPath = args[3];

progress("Copying files to dist...");

copyDir(buildFolder, outputPath);
if (isDebug) {
    makeLink(buildFolder, resolve(__dirname, "../locales"));
} else {
    copyDir(buildFolder, resolve(__dirname, "../locales"));
}


progress("Done")
