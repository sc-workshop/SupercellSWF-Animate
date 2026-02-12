export {};

declare global {
	interface BaseExtension {
		type: "command" | "extension";
		name: string;
		path: string;
		install: string;
	}

	interface Extension extends BaseExtension {
		type: "extension";
		condition?: string;
		variant_name?: string;
	}

	interface CommandExtension extends BaseExtension {
		type: "command";
	}

	interface InstallManifest {
		name: string;
		version: string;
		extensions: BaseExtension[];
	}

	interface ManifestWindow extends Window {
		SupercellSWF?: {
			cwd: string;
			cwd_path: string;

			/**
			 * Path to helper utilities folder
			 */
			binary_path: string;

			os: "WIN" | "MAC";

			/**
			 * Path to package manifest
			 */
			manifest_path: string;
			/**
			 * Parsed package manifest
			 */
			manifest: InstallManifest;

			/**
			 * User manifest. Undefined when user already has installed extension
			 */
			user_manifest_uri: string;
			/**
			 * Parsed manifest that user might have
			 */
			user_manifest?: InstallManifest;

			locale: Localization;
			error_message: string;

			system: () => SystemInfo;
			_system?: SystemInfo;
		};
	}

	var window: ManifestWindow;

	// Polyfills. Some ECMAScript functions not available in Animate 20-21 so just fill with our own functions
	interface String {
		trim(): string;
	}

	interface Array<T> {
		indexOf(element: T, fromIndex?: number): number | -1;
	}
}

declare class Localization {
	get(tid: string): string;
}

declare interface JSONClass {
	parse(data: string): any;
}

// biome-ignore lint/suspicious/noShadowRestrictedNames: Polyfill declaration
declare const JSON: JSONClass;
