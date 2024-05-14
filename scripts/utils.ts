import pc from "picocolors";

import { join } from "path";
import { userInfo, homedir } from "os";
import { symlinkSync, copyFileSync, mkdirSync, existsSync, readdirSync, lstatSync, unlinkSync, rmdirSync } from "fs";
import { Colors, type Formatter } from "picocolors/types";

export const processPath = process.cwd();

export const isDev = getEnv() === 'development';

export const isWindows = process.platform === "win32";
export const isMac = process.platform === "darwin";

type ColorName = keyof Pick<Colors, { [K in keyof Colors]: Colors[K] extends Formatter ? K : never }[keyof Colors]>;

type Environment = "production" | "development"

export function log(val: any) {
    console.log(val)
}

export function error(val: string) {
    progress(val, 'red')
}

export function progress(val: string, color?: ColorName) {
    var c = color ? color : 'yellow'

    console.log(pc[c](val))
}

export function removeFiles(files: string[], basepath: string = "") {
    for (const file of files) {
        var curPath = join(basepath, file)
        if (!existsSync(curPath)) continue;

        if (lstatSync(curPath).isDirectory()) { // recurse
            if (lstatSync(curPath).isSymbolicLink()) {
                unlinkSync(curPath);
            } else {
                removeDirs(curPath)
            }
        } else { // delete file
            unlinkSync(curPath)
        }
    }
}

export function removeDirs(path: string) {
    if (existsSync(path)) {
        const folder = readdirSync(path);
        removeFiles(folder, path);
        rmdirSync(path);
    }
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
        errorMessage = "Process exit with code " + err.status;
    }

    return new Error(errorMessage)
}

export function getEnv(): Environment {
    let env: Environment = 'development'

    const args = process.argv

    if (args.length >= 3) {
        if (args[2] == "production") {
            env = "production";
        }
    }

    return env
}

export function extensionsFolder() {
    if (isWindows) {
        const extensionsPath = userInfo().homedir + '\\AppData\\Roaming\\Adobe\\CEP\\extensions'
        if (existsSync(extensionsPath))
            mkdirSync(extensionsPath, { recursive: true })

        return extensionsPath;
    } else {
        return join(homedir(), 'Library/Application Support/Adobe/CEP/extensions')
    }

}

export function makeLink(src: string, dst: string) {
    progress(`Creating link from \"${src}\" to \"${dst}\"`, "blue")

    symlinkSync(src, dst, "dir")
}

export function getPreferencesFolder() {

}