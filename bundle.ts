import { ConfigInterface } from "./scripts/manifest/interfaces";

export const config: ConfigInterface = {
    organization: "scWorkshop",
    organization_name: "SC Workshop",
    cep_version: "9.0",
    environment: {
        apps: {
            FLPR: "[20.0,99.0]",
        },
        locales: ["All"],
        runtimes: {
            CSXS: "9.0"
        }
    },
    extensions: {
        PublishSettings: {
            type: "extension",
            parent: "Plugin",
            root: "./publisher",
            path: "./index.html",
            params: [
                "--enable-nodejs",
                "--mixed-context"
            ],
            lifecycle: {
                auto_visible: true
            },
            ui: {
                //name: "SCSWF Publisher",
                type: "ModalDialog",
                size: { width: 500, height: 700 }
            }
        },
        Plugin: {
            type: "extension",
            root: "./plugin",
            path: "./fcm.xml",
            ui: {
                //name: "SC Plugin",
                type: "ModalDialog",
                size: { width: 1, height: 1 }
            }
        },
        ImportCommand: {
            type: "command",
            path: "importer/import_sc.jsfl"
        }
    }
}
