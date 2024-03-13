import { distFolder, bundleId } from "./manifest";
import { removeDirs, log } from "./utils";
import { join } from "path"
import { execSync } from "child_process"
import { existsSync, unlinkSync } from "fs"
import cert from "./cert";
import { platform } from "os"

if (existsSync(distFolder)) {
    removeDirs(distFolder);
}

log(`Building production build for package...`)
execSync("npm run build:prod", {stdio: [0, 1, 2]});

const zxpCmd = platform() == "win32" ? `ZXPSignCmd` : `./ZXPSignCmd`;
const cwdDir = join(__dirname, "zxp");
const certPath = join(process.cwd(), "cert.p12");
const output = join(process.cwd(), "dist", `${bundleId}.zxp`);

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
