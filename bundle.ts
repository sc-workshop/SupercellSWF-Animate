import { ConfigInterface } from "./scripts/manifest/interfaces";

export const config: ConfigInterface = {
    organization: "scwmake",
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
            root: "./plugin",
            path: "./fcm.xml",
            ui: {
                //name: "SC Plugin",
                type: "ModalDialog",
                size: { width: 1, height: 1 }
            }
        }
    }
}
