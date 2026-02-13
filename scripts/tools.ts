import which from "which";
import { exec, isMac, progress } from "./utils";

export function findTool(name: string): string | undefined {
	const result = which.sync(name, { nothrow: true, all: false });
	return result !== null ? result : undefined;
}

export function getBrewPath(): string {
	const brew = findTool("brew");
	if (brew === undefined) {
		progress("Trying to install brew manually...");
		exec(
			'/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"',
		);
		return getBrewPath();
	}

	return brew;
}

export function fetchBrewPackage(name: string) {
	const brew = getBrewPath();

	progress(`Fetching ${name} using homebrew...`);
	exec(`${brew} install ${name}`);
}

export function getCmakePath() {
	let path = findTool("cmake");

	if (path === undefined && isMac) {
		fetchBrewPackage("cmake");
		path = findTool("cmake");
	}

	if (path === undefined) throw new Error("Failed to get cmake");

	return path;
}
