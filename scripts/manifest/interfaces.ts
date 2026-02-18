interface EnvironmentInterface {
	apps: { [name: string]: string };
	locales: string[];
	runtimes: { [name: string]: string };
}

type UiType = "Panel" | "Modeless" | "ModalDialog" | "Custom";

interface Size {
	width: number;
	height: number;
}

interface Icon {
	type: string;
	path: string;
}

export interface BaseExtension {
	type: "extension" | "command" | "native";
	root: string;
	children?: string[];
	path: string;
	scriptPath?: string;
	params?: string[];
	lifecycle?: {
		auto_visible: boolean;
		//events: any,
	};
	ui: {
		name?: string;
		type: UiType;
		size: Size;
		maxSize?: Size;
		minSize?: Size;
		icons?: Icon[];
	};
}

export interface CommandExtension {
	type: "command";
	path: string;
}

export interface NativeExtension extends BaseExtension {
	type: "native";
	useFeatureSets?: boolean;
}

export type Extension = BaseExtension | NativeExtension | CommandExtension;

export interface ConfigInterface {
	organization: string; // Name for package signing
	organization_name?: string; // Organization name but more readable
	cep_version: string;
	environment: EnvironmentInterface;
	extensions: {
		[name: string]: Extension;
	};
}
