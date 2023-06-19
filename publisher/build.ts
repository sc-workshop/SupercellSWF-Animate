import { copyDir, makeLink, progress } from "../scripts/utils";
import { join } from "path";
import { existsSync } from "fs";

const args = process.argv;
const buildFolder = join(__dirname, "build");

const isDebug = args[2] === "development";
const outputPath = args[3];

progress("Copying files to dist...");

if (isDebug) {
    if (!existsSync(outputPath)) {
        makeLink(buildFolder, outputPath);
    }

} else {
    copyDir(buildFolder, outputPath);
}

progress("Done")
