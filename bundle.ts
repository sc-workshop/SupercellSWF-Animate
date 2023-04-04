import { ConfigInterface } from "./scripts/manifest/interfaces";

export const config: ConfigInterface = {
    organization: "scwmake",
    cep_version: "9.0",
    environment: {
        apps: {
            FLPR: "15.0",
        },
        locales: ["All"],
        runtimes: {
            CSXS: "9.0"
        }
    },
    extensions: {
        Plugin: {
            root: "./plugin",
            path: "./fcm.xml",
            ui: {
                name: "SC Plugin",
                type: "ModalDialog",
                size: {width: 1, height: 1}
            }
        },
        PublishSettings: {
            root: "./publisher",
            path: "./index.html",
            params: [
                "--enable-nodejs", 
                "--allow-file-access",
                 "--allow-file-access-from-files"
            ],
            ui: {
                name: "SCSWF Publisher",
                type: "ModalDialog",
                size: {width: 350, height: 700}
            }
        }
    }
}
