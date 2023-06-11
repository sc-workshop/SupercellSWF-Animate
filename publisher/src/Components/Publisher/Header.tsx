import { createElement } from "react";
import { Stylefield } from "../Shared/Stylefield";
import { Locale } from "../../Localization";

const Version = require("../../../package.json").version;

export function Header() {

    const headerLine = createElement("vr",
    {
      key: "HeaderInfoLine",
      style: {
        position: "absolute",
        marginLeft: "50%",
        height: "50px",
        borderLeft: "4px solid #484848"
      }
    }
  )

    return createElement("div",
        {
            key: "PublisherHeader",
            style: {
                height: "45px"
            }
        },

        Stylefield("SC",
            {
                color: "white",
                fontSize: "25px",
                fontWeight: "bolder",
                position: "absolute"
            }
        ),

        Stylefield(
            `${Locale.Get("TID_VERSION")} ${Version}`,
            {
                color: "white",
                fontSize: "15px",
                fontWeight: "bolder",
                marginTop: "35px",
                position: "absolute"
            }
        ),

        headerLine,

        
    )
}