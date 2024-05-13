import { copyDir, makeLink, progress } from "../scripts/utils";
import { join } from "path";
import { existsSync } from "fs";

const args = process.argv;
const isDebug = args[2] === "development"
const buildFolder = join(__dirname, isDebug ? "build_debug" : "build");
const outputPath = args[3];

progress(`Copying files to ${outputPath}`)
if (isDebug) {
    if (!existsSync(outputPath)) {
        makeLink(buildFolder, outputPath);
    }

} else {
    copyDir(buildFolder, outputPath);
}

progress("Done")
