import { generateCSXS, config } from "./manifest";
import { progress, isDev } from "./utils";
import { deploy } from "./deploy";
import { build_extension } from "./build/extension";

const extensions_to_build = process.argv.slice(3);

progress(`BUILD for ${isDev ? "Development" : "Production"}`, 'blue');

progress('Generating manifest..');
generateCSXS(config);

for (const extensionName of Object.keys(config.extensions)) {
    if (extensions_to_build.length != 0 && extensions_to_build.indexOf(extensionName) === -1) {
        continue;
    }

    const extension = config.extensions[extensionName];
    progress(`Building ${extensionName}..`);

    switch (extension.type)
    {
        case "extension":
            build_extension(extensionName, extension);
            break;
    }
}

if (isDev)
{
    deploy()
}
