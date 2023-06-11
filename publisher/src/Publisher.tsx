import { createElement } from 'react';
import { BasicSettings } from './Components/Publisher/BasicSettings';
import { publish } from './Components';
import { Stylefield } from './Components/Shared/Stylefield';
import { Header } from './Components/Publisher/Header';

function Publisher() {
  const headerLine = createElement("hr",
    {
      key: "HeaderLine",
      style: {
        width: "99%",
        display: "block",
        border: "2px solid #484848",
      }
    }
  )

  const publishButton = createElement("button",
    {
      style: {
        bottom: "25px",
        width: "100px",
        height: "35px",
        position: "absolute",
        border: "3px solid white",
        borderRadius: "20px",
        background: "rgba(0,0,0,0.0)",
      },
      onClick: publish
    },
    Stylefield("Publish", {
      color: "white",
      textShadow: "1px 1px black"
    }))

  return createElement("div",
    {
      id: "Publisher",
      style: {
        display: 'block'
      },
      children: [
        Header(),
        headerLine,
        BasicSettings(),

        publishButton
      ]
    });
}

export default Publisher;
