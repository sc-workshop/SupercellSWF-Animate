import { config } from "../bundle";
import { readdirSync } from "fs";
import { join, resolve } from "path";
import { progress, removeDirs as removeFiles } from "./utils";

export function cleanup() {
    progress('Cleaning dist files...');
    removeFiles("dist");

    for (const extensionName of Object.keys(config.extensions)) {
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