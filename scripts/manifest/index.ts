import { mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { join as joinPath, posix, resolve as resolvePath } from "node:path";
import { create as CreateXML } from "xmlbuilder2";
import { config } from "../../bundle";
import { version as bundleVersion, name } from "../../package.json";
import { isDev, isExtension, processPath } from "../utils";
import type { ConfigInterface } from "./interfaces";

export { config } from "../../bundle";

const modeName = isDev ? "development" : "production";
export const bundleId = `org.${config.organization}.${name}`;
export const distFolder = joinPath(process.cwd(), "dist", modeName);
export const packageFolder = joinPath(process.cwd(), "dist", "package");
export const extensionDistFolder = joinPath(distFolder, "extension");

export function generateCSXS(config: ConfigInterface) {
	const root = CreateXML({ version: "1.0", encoding: "UTF-8" });
	const debugRoot = CreateXML({ version: "1.0", encoding: "UTF-8" });
	const debugList = debugRoot.ele("ExtensionList");

	const manifest = root.ele("ExtensionManifest", {
		"xmlns:xsi": "http://www.w3.org/2001/XMLSchema-instance",
		Version: "6.0",
		ExtensionBundleId: bundleId,
		ExtensionBundleVersion: bundleVersion,
		ExtensionBundleName: name,
	});

	const extensionList = manifest.ele("ExtensionList");

	const environment = manifest.ele("ExecutionEnvironment");

	const hosts = environment.ele("HostList");
	const apps = Object.keys(config.environment.apps);
	for (const hostName of apps) {
		hosts.ele("Host", {
			Name: hostName,
			Version: config.environment.apps[hostName],
		});
	}

	const locales = environment.ele("LocaleList");
	for (const localeCode of config.environment.locales) {
		locales.ele("Locale", {
			Code: localeCode,
		});
	}

	const runtimes = environment.ele("RequiredRuntimeList");
	for (const runtimeName of Object.keys(config.environment.runtimes)) {
		runtimes.ele("RequiredRuntime", {
			Name: runtimeName,
			Version: config.environment.runtimes[runtimeName],
		});
	}

	const extensions = manifest.ele("DispatchInfoList");

	for (const extensionName of Object.keys(config.extensions)) {
		const extension = config.extensions[extensionName];
		if (!isExtension(extension)) continue;

		const extensionRoot = resolvePath(processPath, extension.root);
		const extensionPackage = JSON.parse(
			readFileSync(joinPath(extensionRoot, "package.json")).toString(),
		);
		const extensionID = `${bundleId}.${extensionName}`;
		const extensionDistRoot = joinPath(distFolder, extensionName);

		if (isDev) {
			const hostsDebugList = debugList
				.ele("Extension", { Id: extensionID })
				.ele("HostList");
			for (let appIndex = 0; apps.length > appIndex; appIndex++) {
				hostsDebugList.ele("Host", {
					Name: apps[appIndex],
					Port:
						3000 +
						appIndex +
						Object.keys(config.extensions).indexOf(extensionName) * 100,
				});
			}
		}

		extensionList.ele("Extension", {
			Id: extensionID,
			Version: extensionPackage.version,
		});

		const extensionNode = extensions
			.ele("Extension", {
				Id: extensionID,
			})
			.ele("DispatchInfo");

		const resources = extensionNode.ele("Resources");

		resources
			.ele("MainPath")
			.txt(
				"./" +
					posix.relative(
						extensionDistRoot,
						posix.join(extensionDistRoot, extensionName, extension.path),
					),
			);

		if (extension.scriptPath != undefined) {
			resources.ele("ScriptPath").txt(extension.scriptPath);
		}

		if (extension.params != undefined) {
			const params = extensionNode.ele("CEFCommandLine");

			for (const param of extension.params) {
				params.ele("Parameter").txt(param);
			}
		}

		if (extension.lifecycle != undefined) {
			const lifecycle = extensionNode.ele("Lifecycle");

			lifecycle
				.ele("AutoVisible")
				.txt(String(extension.lifecycle.auto_visible));
		}

		const ui = extensionNode.ele("UI");

		ui.ele("Type").txt(extension.ui.type);

		if (extension.ui.name) {
			ui.ele("Menu").txt(extension.ui.name);
		}

		const geometry = ui.ele("Geometry");

		const size = geometry.ele("Size");
		size.ele("Height").txt(String(extension.ui.size.height));
		size.ele("Width").txt(String(extension.ui.size.width));

		if (extension.ui.maxSize != undefined) {
			const maxSize = geometry.ele("MaxSize");
			maxSize.ele("Height").txt(String(extension.ui.maxSize.height));
			maxSize.ele("Width").txt(String(extension.ui.maxSize.width));
		}

		if (extension.ui.minSize != undefined) {
			const minSize = geometry.ele("MinSize");
			minSize.ele("Height").txt(String(extension.ui.minSize.height));
			minSize.ele("Width").txt(String(extension.ui.minSize.width));
		}

		if (extension.ui.icons != undefined) {
			const icons = ui.ele("Icons");
			for (const icon of extension.ui.icons) {
				icons
					.ele("Icon", {
						Type: icon.type,
					})
					.txt(icon.path);
			}
		}
	}

	const outputFolder = joinPath(extensionDistFolder, "CSXS");
	mkdirSync(outputFolder, { recursive: true });
	writeFileSync(
		joinPath(outputFolder, "manifest.xml"),
		root.toString({ prettyPrint: true }),
		"utf-8",
	);

	if (isDev) {
		writeFileSync(
			joinPath(extensionDistFolder, ".debug"),
			debugRoot.toString({ prettyPrint: true }),
			"utf-8",
		);
	}
}
