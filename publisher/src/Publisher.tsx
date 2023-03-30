import { createElement } from 'react';
import { BasicSettings } from './Components/Publisher/BasicSettings';
import { theme, themes } from './Components/themes';
import { onLoad, publish } from './Components';
import { Stylefield } from './Components/Shared/Stylefield';
import { Header } from './Components/Publisher/Header';

function Publisher() {
  onLoad();

  const publishButton = createElement("button", 
  {
    style: {
      marginLeft: "1vh",
      bottom: "25px",
      height: "25px",
      width: "100%",
      position: "absolute",
      border: "1px solid #070707",
      background:"#444444",
      padding: "4px",
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
      style: {
        backgroundColor: theme === themes.Dark ? "#333333" : "#FFFFFF",
        margin: "10px",
        width: "100vw",
        height: "100vh",
        fontSize: "12px"
      },
      children: [
        Header(),
        BasicSettings(),
        publishButton
      ]
    });
}

export default Publisher;
