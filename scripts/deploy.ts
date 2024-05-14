import { copyDir, extensionsFolder, error, isDev, isWindows, processExecError, progress, removeFiles, makeLink } from "./utils";
import { execSync } from "child_process"
import { bundleId, config, extensionDistFolder } from "./manifest";
import winreg from 'registry-js'

import { join } from "path";

const deployFolder = join(extensionsFolder(), bundleId)

export function deploy() {
    progress(`DEPLOY for ${isDev ? "Development" : "Production"}`, 'blue');

    progress('Cleaning deploy target');

    try {
        removeFiles([deployFolder]);
    } catch (err) {
        error(err as any)
    }

    if (isDev) {
        try {
            const csxsVersion = config.cep_version.split(".")[0];

            progress(`PlayerDebug for CEP ${csxsVersion} patching`);

            if (isWindows) {
                const subkey = `Software\\Adobe\\CSXS.${csxsVersion}`;
                winreg.createKeySafe(winreg.HKEY.HKEY_CURRENT_USER, subkey);
                winreg.setValueSafe(winreg.HKEY.HKEY_CURRENT_USER, subkey, "PlayerDebugMode", winreg.RegistryValueType.REG_SZ, "1");
            } else {
                execSync(`defaults write com.adobe.CSXS.${csxsVersion} PlayerDebugMode 1`, { stdio: [0, 1, 2] })
            }
        } catch (err) {
            processExecError(err as any)
        }

        progress('Creating symlink into extensions folder')

        try {
            makeLink(extensionDistFolder, deployFolder);
        } catch (err) {
            error(err as any)
        }

    } else {
        progress('Copying into extensions folder')
        try {
            copyDir(extensionDistFolder, deployFolder);

        } catch (err) {
            error(err as any)
        }
    }
}

if (require.main === module)
{
    deploy();
}

