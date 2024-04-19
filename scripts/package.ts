import { distFolder, bundleId } from "./manifest";
import { removeDirs, log, isWindows } from "./utils";
import { join } from "path"
import { execSync } from "child_process"
import { existsSync, unlinkSync } from "fs"
import cert from "./cert";
import { platform } from "os"
import { version } from "../package.json"

const rebuild_package = process.argv.indexOf("--raw-package") == -1 || !existsSync(distFolder)

if (rebuild_package)
{
    if (existsSync(distFolder)) {
        removeDirs(distFolder);
    }
}

if (rebuild_package)
{
    log(`Building production build for package...`)
    execSync("npm run build:prod", {stdio: [0, 1, 2]});    
}

const zxpCmd = isWindows ? `ZXPSignCmd` : `./ZXPSignCmd`;
const cwdDir = join(__dirname, "zxp");
const certPath = join(process.cwd(), "cert.p12");
const platform_prefix = isWindows ? "Windows" : "MacOS";
const output = join(process.cwd(), "dist", `${bundleId}-${platform_prefix}-${version}.zxp`);

if (existsSync(certPath)) {
    unlinkSync(certPath)
}

if (existsSync(output)) {
    unlinkSync(output);
}

execSync(`${zxpCmd} -selfSignedCert ${cert.countryCode} ${cert.province} ${cert.organization} ${bundleId} ${cert.password} "${certPath}"`, {cwd: cwdDir, stdio:[0, 1, 2]})

execSync(`${zxpCmd} -sign "${distFolder}" "${output}" "${certPath}" ${cert.password} -tsa http://timestamp.digicert.com/`, {cwd: cwdDir, stdio:[0, 1, 2]})

log(`Package saved to ${output}`)

removeDirs(join(distFolder, bundleId))
