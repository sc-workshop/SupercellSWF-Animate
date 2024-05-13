import { config } from "../bundle";
import { readdirSync, existsSync } from "fs";
import { join, resolve } from "path";
import { progress, removeDirs as removeFiles } from "./utils";
import { distFolder } from "./manifest";

export function cleanup(extensions: string[] = []) {
    progress('Cleaning dist files...');
    if (existsSync(distFolder)) {
        removeFiles(distFolder)
    }

    for (const extensionName of Object.keys(config.extensions)) {
        if (extensions.length != 0) {
            if (extensions.indexOf(extensionName) === -1) {
                continue;
            }
        }
        const extension = config.extensions[extensionName];

        const content = readdirSync(extension.root);

        if (content.includes(".dist")) {
            const distFile = join(extension.root, ".dist");
            const distFiles = distFile.split(/\r?\n/);

            for (const file of distFiles) {
                if (file.startsWith("#")) {
                    continue;
                }

                removeFiles(resolve(extension.root, file));
            }
        }


    }
}