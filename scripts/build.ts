import { generateCSXS, config } from "./manifest";
import { progress, isDev } from "./utils";
import { deploy } from "./deploy";
import { build_extension } from "./build/extension";

const args = process.argv.slice(3);
let extensionsToBuild: String[] = [];
let flags: String[] = [];
for (let arg of args)
{
    if (arg.startsWith("--"))
    {
        flags.push(arg);
    } else
    {
        extensionsToBuild.push(arg);
    }
}

progress(`BUILD for ${isDev ? "Development" : "Production"}`, 'blue');

progress('Generating manifest..');
generateCSXS(config);

for (const extensionName of Object.keys(config.extensions)) {
    if (extensionsToBuild.length != 0 && extensionsToBuild.indexOf(extensionName) === -1) {
        continue;
    }

    const extension = config.extensions[extensionName];
    progress(`Building ${extensionName}..`);
    

    switch (extension.type)
    {
        case "extension":
            build_extension(extensionName, extension, flags);
            break;
    }
}

if (isDev)
{
    deploy()
}
