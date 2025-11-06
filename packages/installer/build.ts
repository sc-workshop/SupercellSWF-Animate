import { execSync } from "child_process";
import { copyDir } from "../../scripts/utils";
import { lstatSync, readdirSync, renameSync } from "fs";
import { format, join, parse } from "path";

const args = process.argv;
const outputPath = args[2];
copyDir("./runtime", outputPath)

execSync(`tsc --outDir \"${outputPath}\"`, {stdio: [0, 1, 2], cwd: __dirname})

function rename_files(path: string)
{
    if (lstatSync(path).isDirectory())
    {
        const directory = readdirSync(path);
        for (const file of directory)
        {
            rename_files(join(path, file))
        }
    } else 
    {
        if (path.endsWith(".js"))
        {
            renameSync(
                path,
                format({ ...parse(path), base: '', ext: '.jsfl' })
            )
        }
    }
}
rename_files(outputPath);