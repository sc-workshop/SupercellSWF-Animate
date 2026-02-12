import { execSync } from "node:child_process";
import { existsSync, lstatSync, readdirSync, renameSync } from "node:fs";
import { format, join, parse } from "node:path";
import { copyDir } from "../../scripts/utils";

const args = process.argv;
const outputPath = args[2];
const binariesPath = join(__dirname, "bin", process.platform);
const runtimePath = join(__dirname, "runtime");
if (!existsSync(binariesPath))
	throw new Error("Unsupported system");

copyDir(runtimePath, outputPath);
copyDir(binariesPath, join(outputPath, "bin"));

execSync(`tsc --outDir "${outputPath}"`, { stdio: [0, 1, 2], cwd: __dirname });

function rename_files(path: string) {
	if (lstatSync(path).isDirectory()) {
		const directory = readdirSync(path);
		for (const file of directory) {
			rename_files(join(path, file));
		}
	} else {
		if (path.endsWith(".js")) {
			renameSync(path, format({ ...parse(path), base: "", ext: ".jsfl" }));
		}
	}
}
rename_files(outputPath);
