import { join, resolve } from "path";
import { extensionDistFolder } from "../manifest";
import { CommandExtension } from "../manifest/interfaces";
import { isDev, processExecError, processPath, removeFiles } from "../utils";
import { execSync } from "child_process"
import { existsSync } from "fs";

export function build_command(extension: CommandExtension)
{
    //const outputFolder = join(extensionDistFolder, name);
    //if (existsSync(outputFolder))
    //{
    //    removeFiles([outputFolder])
    //}
//
    //try {
    //    execSync(`npm run build:${isDev ? "dev" : "prod"} -- "${outputFolder}" ${flags.join(" ")}`, {stdio: [0, 1, 2], cwd: resolve(processPath, extension.root)});
    //} catch (err) {
    //    throw processExecError(err as any);
    //}
}