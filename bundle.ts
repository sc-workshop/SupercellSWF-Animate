import type { ConfigInterface } from "./scripts/manifest/interfaces";

export const config: ConfigInterface = {
	organization: "scWorkshop",
	organization_name: "SC Workshop",
	cep_version: "9.0",
	environment: {
		apps: {
			FLPR: "[19.0,99.0]",
		},
		locales: ["All"],
		runtimes: {
			CSXS: "9.0",
		},
	},
	extensions: {
		Plugin: {
			type: "native",
			root: "./packages/plugin",
			path: "./fcm.xml",
			children: ["PublishSettings"],
			useFeatureSets: true,
			ui: {
				//name: "SC Plugin",
				type: "ModalDialog",
				size: { width: 1, height: 1 },
			},
		},
		PublishSettings: {
			type: "extension",
			root: "./packages/publisher",
			path: "./index.html",
			params: ["--enable-nodejs", "--mixed-context"],
			lifecycle: {
				auto_visible: true,
			},
			ui: {
				//name: "SCSWF Publisher",
				type: "ModalDialog",
				size: { width: 630, height: 800 },
			},
		},
		/*ExportNameTransfer:
        {
            type: "command",
            path: "jsfl/export_name_transfer.jsfl"
        }*/
		// ImportCommand: {
		//     type: "command",
		//     name: "SupercellSWF/Import .sc",
		//     path: "importer/import_sc.jsfl"
		// }
	},
};
