import { spawnSync } from "node:child_process";
import {
	existsSync,
	mkdirSync,
	readdirSync,
	rmSync,
} from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const packageRoot = path.resolve(
	path.dirname(fileURLToPath(import.meta.url)),
	"..",
);
const schemaDir = path.join(
	packageRoot,
	"supercell-flash-plugin",
	"schemas",
);
const outputDir = path.join(
	packageRoot,
	"supercell-flash-plugin",
	"source",
	"Cache",
	"Generated",
);

function findFlatc() {
	const explicit = process.env.FLATC;
	if (explicit && existsSync(explicit)) {
		return explicit;
	}

	const executable = process.platform === "win32" ? "flatc.exe" : "flatc";
	const installRoots = [
		process.env.VCPKG_INSTALLED_DIR,
		path.join(packageRoot, "build", "vcpkg_installed"),
		path.join(packageRoot, "build-release", "vcpkg_installed"),
	].filter(Boolean);

	for (const installRoot of installRoots) {
		if (!existsSync(installRoot)) {
			continue;
		}

		for (const entry of readdirSync(installRoot, { withFileTypes: true })) {
			if (!entry.isDirectory()) {
				continue;
			}

			const candidate = path.join(
				installRoot,
				entry.name,
				"tools",
				"flatbuffers",
				executable,
			);
			if (existsSync(candidate)) {
				return candidate;
			}
		}
	}

	throw new Error(
		[
			"flatc not found.",
			"Install it with vcpkg, for example:",
			"  vcpkg install flatbuffers:x64-windows --x-install-root build\\vcpkg_installed",
			"Or set FLATC to the flatc executable path.",
		].join("\n"),
	);
}

const flatc = findFlatc();
const schemas = readdirSync(schemaDir)
	.filter((name) => name.endsWith(".fbs"))
	.sort()
	.map((name) => path.join(schemaDir, name));

if (schemas.length === 0) {
	throw new Error(`No .fbs files found in ${schemaDir}`);
}

rmSync(outputDir, { recursive: true, force: true });
mkdirSync(outputDir, { recursive: true });

const args = [
	"--cpp",
	"--cpp-std",
	"c++17",
	"--scoped-enums",
	"-o",
	outputDir,
	...schemas,
];

console.log(`Generating FlatBuffers headers with ${flatc}`);
const result = spawnSync(flatc, args, { stdio: "inherit" });
if (result.status !== 0) {
	throw new Error(`flatc failed with exit code ${result.status}`);
}

console.log(`Generated headers in ${outputDir}`);
