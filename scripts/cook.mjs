import { execSync } from "child_process";
import { join } from "path";
import { cwd } from "process";

const installer_cwd = join(cwd(), "installer");
const publisher_cwd = join(cwd(), "publisher");
const plugin_cwd = join(cwd(), "plugin");

const install_paths = [installer_cwd, publisher_cwd, plugin_cwd];

for (const path of install_paths)
{
    execSync("npm install --save-dev", { stdio: [0, 1, 2], cwd: path })
}