import { createElement } from 'react';
import { publish } from './Components';
import TextField from './Components/Shared/TextField';
import { Header } from './Components/Publisher/Header';
import Settings from './Components/Publisher/Settings';
import BasicSettings from './Components/Publisher/BasicSettings';
import Locale from './Localization';
import FileField from './Components/Shared/FileField';
import { State } from './Components/publisherState';

function Publisher() {
  const delim = <hr key="header_delin" style={{
    width: "99%",
    border: "2px solid #484848",
  }} />;

  const publishButtonLabel = TextField(
    "Publish",
    {
      color: "white",
      textShadow: "1px 1px black"
    }
  );

  const publishButton = createElement("button",
    {
      key: "publish_button",
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
    publishButtonLabel
  )

  return createElement("div",
    {
      id: "publisher",
      style: {

      },
      children: [
        Header(),

        delim,

        BasicSettings(),
        Settings(),

        publishButton
      ]
    });
}

export default Publisher;
