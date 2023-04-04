import { copyDir, extensionsFolder, error, isDev, isWindows, processExecError, progress, removeDirs } from "./utils";
import { existsSync, lstatSync, unlinkSync } from "fs";
import { execSync } from "child_process"
import { bundleId, config, distFolder } from "./manifest";
import { symlinkSync } from "fs";
import { join } from "path";

const deployFolder = join(extensionsFolder(), bundleId)

progress(`DEPLOY for ${isDev ? "Development" : "Production"}`, 'blue');

progress('Cleaning deploy target');

try {
    if (existsSync(deployFolder) && lstatSync(deployFolder).isSymbolicLink()) {
        unlinkSync(deployFolder)
    }
    removeDirs(deployFolder)
} catch (err) {
    error(err as any)
}

if (isDev) {
    try {
        const csxsVersion = config.cep_version.split(".")[0];

        progress(`PlayerDebug for CEP ${csxsVersion} patching`);

        if (isWindows) {
            execSync(`REG ADD HKEY_CURRENT_USER\\Software\\Adobe\\CSXS.${csxsVersion} /v PlayerDebugMode /t REG_SZ /d 1 /f`)
        } else {
            execSync(`defaults write com.adobe.CSXS.${csxsVersion} PlayerDebugMode 1`, { stdio: [0, 1, 2] })
        }
    } catch (err) {
        processExecError(err as any)
    }

    progress('Creating symlink into extensions folder')

    try {
        const type = isWindows ? 'junction' : 'dir'

        symlinkSync(distFolder, deployFolder, type)
    } catch (err) {
        error(err as any)
    }

} else {
    progress('Copying into extensions folder')
    try {
        copyDir(distFolder, deployFolder);

    } catch (err) {
        error(err as any)
    }
}
