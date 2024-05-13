import { generateCSXS, config, distFolder } from "./manifest";
import { progress, isDev, processPath, processExecError } from "./utils";
import { join, resolve } from "path"
import { execSync } from "child_process"
import { deploy } from "./deploy";

const extensions = process.argv.slice(3);

progress(`BUILD for ${isDev ? "Development" : "Production"}`, 'blue');

progress('Generating manifest..');
generateCSXS(config);

for (const extensionName of Object.keys(config.extensions)) {
    if (extensions.length != 0 && extensions.indexOf(extensionName) === -1) {
        continue;
    }
    progress(`Building ${extensionName}..`);

    const extensionDistFolder = join(distFolder, extensionName);
    try {
        execSync(`npm run build:${isDev ? "dev" : "prod"} -- "${extensionDistFolder}"`, {stdio: [0, 1, 2], cwd: resolve(processPath, config.extensions[extensionName].root)});
    } catch (err) {
        throw processExecError(err as any);
    }
}

if (isDev)
{
    deploy()
}
