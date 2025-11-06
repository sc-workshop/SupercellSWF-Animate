import { existsSync, readdirSync } from "node:fs";
import { join, resolve } from "node:path";
import { config } from "../bundle";
import { distFolder } from "./manifest";
import type { Extension } from "./manifest/interfaces";
import { progress, removeDirs as removeFiles } from "./utils";

export function cleanup_extension(extension: Extension) {
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

export function cleanup(extensions: string[] = []) {
	progress("Cleaning dist files...");
	if (existsSync(distFolder)) {
		removeFiles(distFolder);
	}

	for (const extensionName of Object.keys(config.extensions)) {
		if (extensions.length != 0) {
			if (extensions.indexOf(extensionName) === -1) {
				continue;
			}
		}
		const extension = config.extensions[extensionName];
		switch (extension.type) {
			case "extension":
				cleanup_extension(extension);
				break;
			default:
				break;
		}
		// TODO
	}
}
