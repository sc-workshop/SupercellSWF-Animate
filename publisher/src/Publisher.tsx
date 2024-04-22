import { createElement } from 'react';
import { publish } from './Components';
import { Header } from './Components/Publisher/Header';
import SettingsMenu from './Components/Publisher/Settings';
import BasicSettings from './Components/Publisher/BasicSettings';
import Button from './Components/Shared/Button';
import Locale, { Locales } from './Localization';
import EnumField from './Components/Shared/EnumField';
import { loadFont } from '.';

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

  const available_languages = ["en_US", "ru_RU"];
  const language = new EnumField({
    name: "Language",
    keyName: "language_debug_sect",
    enumeration: available_languages,
    defaultValue: available_languages[0],
    style: {
      display: "flex",
      alignItems: "center"
    },
    callback: value => {
      const intValue = parseInt(value);
      const localeName = available_languages[intValue];
      const localeCode = Object.entries(Locales).find(([key, val]) => key === localeName)?.[1];
      if (localeCode !== undefined) {
        Locale.code = localeCode;
        Locale.Load();
        loadFont(() => {})
      }
    },
  })

  const buttonContainer = createElement(
    "div",
    {
      key: "button_container",
      style: {
        background: "rgba(25,25,25,255)",
        width: "100%",
        height: "10%",
        display: "flex",
        bottom: "0",
        left: "0",
        position: "fixed",
        alignItems: "center",
      }
    },
    publishButton,
    process.env.NODE_ENV == "production" ? undefined : language.render()
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
        SettingsMenu(),

        buttonContainer
      ]
    });
}

export default Publisher;
