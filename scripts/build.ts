import { cleanup } from "./cleanup";
import { generateCSXS, config, distFolder } from "./manifest";
import { error, progress, isDev, processPath, processExecError } from "./utils";
import { join, resolve } from "path"
import { mkdirSync } from "fs"
import { execSync } from "child_process"

progress(`BUILD for ${isDev ? "Development" : "Production"}`, 'blue');
cleanup();

progress('Generating manifest..');
generateCSXS(config);

for (const extensionName of Object.keys(config.extensions)) {
    progress(`Building ${extensionName}..`);

    const extensionDistFolder = join(distFolder, extensionName);
    mkdirSync(extensionDistFolder, { recursive: true });
    process.chdir(resolve(processPath, config.extensions[extensionName].root));
    try {
        execSync(`npm run build:${isDev ? "dev" : "prod"} -- "${extensionDistFolder}"`);
    } catch (err) {
        processExecError(err as any);
        error(`Failed to build ${extensionName}`);
        process.exit(1);
    }
}


