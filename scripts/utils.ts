import pc from "picocolors";

import { join } from "path";
import { userInfo, homedir } from "os";
import { copyFileSync, mkdirSync, existsSync, readdirSync, lstatSync, unlinkSync, rmdirSync } from "fs";
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

export function removeDirs(path: string) {
    if (existsSync(path)) {
        const folder = readdirSync(path);

        for (const file of folder) {
            var curPath = join(path, file)
            if (lstatSync(curPath).isDirectory()) { // recurse
                removeDirs(curPath)
            } else { // delete file
                unlinkSync(curPath)
            }
        }

        rmdirSync(path)
    }
}

export function copyDir(src: string, dst: string) {
    if (!existsSync(src) || !lstatSync(src).isDirectory()) {
        return;
    }

    if (!existsSync(dst) || !lstatSync(dst).isDirectory()) {
        mkdirSync(dst, {recursive: true});
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

export function processExecError(err: any) {
    if (err.status !== undefined) {
        error("Process exit with code " + err.status)
    }

    if (err.stdout.length !== 0) {
        console.log(err.stdout.toString());
    }

    if (err.stderr.length !== 0) {
        console.log(err.stderr.toString());
    }
    
    console.log(err.message);
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
