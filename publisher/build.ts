import { copyDir, progress } from "../scripts/utils";
import { join } from "path";

const args = process.argv;
const buildFolder = join(__dirname, "build");

const outputPath = args[3];

progress("Copying files to dist...");

copyDir(buildFolder, outputPath);

progress("Done")
