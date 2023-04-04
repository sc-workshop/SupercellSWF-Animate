import { createElement } from 'react';
import { BasicSettings } from './Components/Publisher/BasicSettings';
import { onLoad, publish } from './Components';
import { Stylefield } from './Components/Shared/Stylefield';
import { Header } from './Components/Publisher/Header';

function Publisher() {
  onLoad();

  const publishButton = createElement("button", 
  {
    style: {
      bottom: "25px",
      height: "25px",
      width: "20%",
      position: "absolute",
      border: "1px solid #070707",
      background:"#444444",
      color: "#c6c6c6"
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
      children: [
        Header(),
        BasicSettings(),
        publishButton
      ]
    });
}

export default Publisher;
