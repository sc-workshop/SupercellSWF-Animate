import { createElement } from "react";
import TextField from "Components/Shared/TextField";
import Locale from "Localization";

// eslint-disable-next-line @typescript-eslint/no-var-requires
const Version = require("../../../package.json").version;

export function Header() {

    const delim = createElement("vr",
        {
            key: "header_info_delim",
            style: {
                position: "absolute",
                marginLeft: "50%",
                height: "60px",
                borderLeft: "4px solid #484848"
            }
        }
    )

    const label = TextField(
        `SC`,
        {
            color: "white",
            fontSize: "30px",
            position: "absolute"
        },
        "header_info_title"
    )

    const versionLabel = TextField(
        `${Locale.Get("TID_VERSION")} ${Version}`,
        {
            color: "white",
            fontSize: "20px",
            marginTop: "45px",
            position: "absolute"
        },
        "header_info_version"
    )

    return createElement(
        "div",
        {
            key: "publisher_info",
            style: {
                height: "60px"
            }
        },
        label,
        versionLabel,
        delim
    )
}