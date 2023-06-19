import { cleanup } from "./cleanup";
import { generateCSXS, config, distFolder } from "./manifest";
import { progress, isDev, processPath, processExecError } from "./utils";
import { join, resolve } from "path"
import { execSync } from "child_process"

const extensions = process.argv.slice(3, process.argv.length);

progress(`BUILD for ${isDev ? "Development" : "Production"}`, 'blue');
cleanup(extensions);

progress('Generating manifest..');
generateCSXS(config);

for (const extensionName of Object.keys(config.extensions)) {
    if (extensions.length != 0) {
        if (extensions.indexOf(extensionName) === -1) {
            continue;
        } 
    }
    progress(`Building ${extensionName}..`);

    const extensionDistFolder = join(distFolder, extensionName);
    process.chdir(resolve(processPath, config.extensions[extensionName].root));
    try {
        execSync(`npm run build:${isDev ? "dev" : "prod"} -- "${extensionDistFolder}"`, {stdio: [0, 1, 2]});
    } catch (err) {
        throw processExecError(err as any);
    }
}


