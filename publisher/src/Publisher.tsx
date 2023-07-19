import { createElement } from 'react';
import { publish } from './Components';
import { Header } from './Components/Publisher/Header';
import Settings from './Components/Publisher/Settings';
import BasicSettings from './Components/Publisher/BasicSettings';
import Button from './Components/Shared/Button';
import Locale from './Localization';

function Publisher() {
  const delim = <hr key="header_delim" style={{
    width: "99%",
    border: "2px solid #484848",
  }} />;

  const publishButton = Button(
    Locale.Get("TID_PUBLISH"),
    "publish_start",
    {
      margin: "10px"
    },
    publish
  );

  const buttonContainer = createElement(
    "div",
    {
      key: "button_container",
      style: {
        background: "rgba(25,25,25,255)",
        width: "100%",
        height: "10%",
        display: "flex",
        flexDirection: "column",
        bottom: "0",
        left: "0",
        position: "fixed"
      }
    },
    publishButton
  )

  return createElement("div",
    {
      id: "publisher",
      style: {
        width: "100%",
        height: "100%"
      },
      children: [
        Header(),

        delim,

        BasicSettings(),
        Settings(),

        buttonContainer
      ]
    });
}

export default Publisher;
