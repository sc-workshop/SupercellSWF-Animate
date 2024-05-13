interface EnvironmentInterface {
    apps: { [name: string]: string },
    locales: string[],
    runtimes: { [name: string]: string }
}

type UiType = "Panel" | "Modeless" | "ModalDialog" | "Custom"

interface Size {
    width: number
    height: number
}

interface Icon {
    type: string
    path: string
}

export interface Extension {
    type: "extension"
    root: string
    path: string
    scriptPath?: string
    params?: string[]
    lifecycle?: {
        auto_visible: boolean,
        //events: any,
    }
    ui: {
        name?: string
        type: UiType
        size: Size
        maxSize?: Size,
        minSize?: Size,
        icons?: Icon[]
    }
}

export interface ConfigInterface {
    organization: string
    cep_version: string
    environment: EnvironmentInterface
    extensions: {
        [name: string]: Extension
    }
}