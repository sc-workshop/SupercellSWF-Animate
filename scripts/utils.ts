import { execSync } from "node:child_process";
import {
	copyFileSync,
	existsSync,
	lstatSync,
	mkdirSync,
	readdirSync,
	rmSync,
	symlinkSync,
	unlinkSync,
	writeFileSync,
} from "node:fs";
import { homedir, userInfo } from "node:os";
import { join } from "node:path";
import colors from "picocolors";
import type { Colors, Formatter } from "picocolors/types";
import type { BaseExtension, Extension } from "./manifest/interfaces";

export const processPath = process.cwd();

export const isDev = getEnv() === "development";

export const isWindows = process.platform === "win32";
export const isMac = process.platform === "darwin";

type ColorName = keyof Pick<
	Colors,
	{ [K in keyof Colors]: Colors[K] extends Formatter ? K : never }[keyof Colors]
>;

type Environment = "production" | "development";

export function log(val: any) {
	progress(val, "green");
}

export function error(val: string) {
	progress(val, "red");
}

export function warn(val: string) {
	progress(val, "yellow");
}

export function progress(value: string, color?: ColorName) {
	const destination = color ? color : "yellow";

	console.log(colors[destination](value));
}

export function removeFiles(files: string[], basepath: string = "") {
	for (const file of files) {
		const curPath = join(basepath, file);
		if (!existsSync(curPath)) continue;

		if (lstatSync(curPath).isDirectory()) {
			// recurse
			if (lstatSync(curPath).isSymbolicLink()) {
				unlinkSync(curPath);
			} else {
				removeDirs(curPath);
			}
		} else {
			// delete file
			unlinkSync(curPath);
		}
	}
}

export function removeDirs(path: string) {
	rmSync(path, { recursive: true, force: true });
}

export function copyDir(src: string, dst: string) {
	if (!existsSync(src) || !lstatSync(src).isDirectory()) {
		return;
	}

	if (!existsSync(dst) || !lstatSync(dst).isDirectory()) {
		mkdirSync(dst, { recursive: true });
	}

	for (const name of readdirSync(src)) {
		const path = join(src, name);
		const outPath = join(dst, name);

		if (lstatSync(path).isDirectory()) {
			mkdirSync(outPath, { recursive: true });
			copyDir(path, outPath);
		} else {
			copyFileSync(path, outPath);
		}
	}
}

export function processExecError(err: any): Error {
	let errorMessage = "Failed to exec process";
	if (err.status) {
		errorMessage = `Process exit with code ${err.status}`;
	}

	return new Error(errorMessage);
}

export function getEnv(): Environment {
	let env: Environment = "development";

	const args = process.argv;

	if (args.length >= 3) {
		if (args[2] == "production") {
			env = "production";
		}
	}

	return env;
}

export function extensionsFolder() {
	let folder = "";

	if (isWindows) {
		folder = `${userInfo().homedir}\\AppData\\Roaming\\Adobe\\CEP\\extensions`;
	} else if (isMac) {
		folder = join(homedir(), "Library/Application Support/Adobe/CEP/extensions");
	} else {
		throw new Error("Unsupported platform");
	}

	mkdirSync(folder, { recursive: true });
	return folder;
}

export function makeLink(src: string, dst: string) {
	progress(`Creating link from "${src}" to "${dst}"`, "blue");
	if (existsSync(dst)) {
		warn(`makeLink: ${dst} already exists`);
		return;
	}
	
	symlinkSync(src, dst, "dir");
}

/**
 * Executes command from project root
 * @param command 
 */
export function exec(command: string) {
	execSync(command, { stdio: [0, 1, 2], cwd: join(__dirname, "../") });
}

export function getPreferencesFolder() { }

export function createDistMark(path: string) {
	const dest = join(path, ".dist");
	writeFileSync(dest, "");
}

export function isExtension(extension: Extension): extension is BaseExtension {
	return extension.type !== undefined && (extension.type == "extension" || extension.type == "native");
}
