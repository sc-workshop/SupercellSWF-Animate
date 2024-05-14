import { extensionDistFolder, bundleId, packageFolder, config } from "./manifest";
import { removeDirs, log, isWindows, copyDir } from "./utils";
import { basename, join, relative } from "path/posix"
import { execSync } from "child_process"
import { copyFileSync, existsSync, mkdirSync, unlinkSync, writeFileSync } from 'fs';
import cert from "./cert";
import { version } from "../package.json"
import { cwd } from "process";

function build_extension_package(output_package: string) {
    const rebuild_package = process.argv.indexOf("--raw-package") == -1 || !existsSync(extensionDistFolder)

    if (rebuild_package) {
        if (existsSync(extensionDistFolder)) {
            removeDirs(extensionDistFolder);
        }
    }

    if (rebuild_package) {
        log(`Building production build for package...`)
        execSync("npm run build:prod", { stdio: [0, 1, 2] });
    }

    const zxpCmd = isWindows ? `ZXPSignCmd` : `./ZXPSignCmd`;
    const cwdDir = join(__dirname, "zxp");
    const certPath = join(process.cwd(), "cert.p12");

    if (existsSync(certPath)) {
        unlinkSync(certPath)
    }

    execSync(`${zxpCmd} -selfSignedCert ${cert.countryCode} ${cert.province} ${cert.organization} ${bundleId} ${cert.password} "${certPath}"`, { cwd: cwdDir, stdio: [0, 1, 2] })

    execSync(`${zxpCmd} -sign "${extensionDistFolder}" "${output_package}" "${certPath}" ${cert.password} -tsa http://timestamp.digicert.com/`, { cwd: cwdDir, stdio: [0, 1, 2] })

    log(`Package saved to ${output_package}`)

    removeDirs(join(extensionDistFolder, bundleId))
}

if (existsSync(packageFolder)) {
    removeDirs(packageFolder);
    mkdirSync(packageFolder, { recursive: true })
}

// Copying installer runtime
execSync(`npm run build -- ${packageFolder}`, {stdio: [0, 1, 2], cwd: join(cwd(), "installer")})

const packageDistFolder = join(packageFolder, "packages");
const packageCommandsFolder = join(packageDistFolder, "commands");
mkdirSync(packageDistFolder, { recursive: true })
mkdirSync(packageCommandsFolder, { recursive: true })

// Building extension
const package_name = `${bundleId}-${version}`
const extensionPackagePath = join(packageDistFolder, `${package_name}.zxp`)
build_extension_package(extensionPackagePath);

const packageManifest: any = {
    version: version,
    extensions: [
        {
            type: "extension",
            name: "Plugin",
            path: relative(packageFolder, extensionPackagePath),
            install: bundleId
        }
    ]
}

const scripInstallName = config.organization_name ? config.organization_name : config.organization;

for (const extensionName of Object.keys(config.extensions)) {
    const extension = config.extensions[extensionName];
    if (extension.type === "extension") continue;

    switch (extension.type) {
        case "command":
            const scriptName = basename(extension.path);
            const scriptDist = join(packageCommandsFolder, scriptName);
            copyFileSync(extension.path, scriptDist);

            packageManifest.extensions.push(
                {
                    type: "command",
                    name: extensionName,
                    path: relative(packageFolder, scriptDist),
                    install: `${scripInstallName}/${scriptName}`
                }
            )
            break;
    }
}

writeFileSync(join(packageFolder, "manifest.json"), JSON.stringify(packageManifest))