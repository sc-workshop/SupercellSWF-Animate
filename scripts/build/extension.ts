import { execSync } from "node:child_process";
import { existsSync } from "node:fs";
import { join, resolve } from "node:path";
import { extensionDistFolder } from "../manifest";
import type { BaseExtension } from "../manifest/interfaces";
import { isDev, processExecError, processPath, removeFiles } from "../utils";

export function build_extension(
	name: string,
	extension: BaseExtension,
	flags: string[],
) {
	const outputFolder = join(extensionDistFolder, name);
	if (existsSync(outputFolder)) {
		removeFiles([outputFolder]);
	}

	const script = isDev ? "dev" : "prod";

	try {
		execSync(`pnpm build:${script} "${outputFolder}" ${flags.join(" ")}`, {
			stdio: [0, 1, 2],
			cwd: resolve(processPath, extension.root),
		});
	} catch (err) {
		throw processExecError(err as any);
	}
}
