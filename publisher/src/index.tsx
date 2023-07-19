import ReactDOM from 'react-dom/client';
import Publisher from './Publisher';
import React, { useState, createElement, useEffect } from 'react';
import { AppColor } from './Components/themes';
import { getInterface, CSEvent, isCEP } from './CEP';
import { State } from './Components/publisherState';
import Locale from './Localization';

function App() {
  const [publisherStateData, setPublisherStateData] = useState<string | undefined>(undefined);
  const [isFontLoaded, setIsFontLoaded] = useState(false);

  useEffect(() => {
    // Font
    const loadFont = async () => {
      const font = new FontFace("PublisherFont", `url(${require("./fonts/" + Locale.code)})`, {
        style: "normal",
      });
      await font.load();
      if (font.status == "loaded") {
        document.fonts.add(font);
      }
      setIsFontLoaded(true);
    }
    loadFont();

    // Publisher Data
    if (!isCEP()) {
      setPublisherStateData("{}");
      return;
    };

    const CSInterface = getInterface();
    const getData = async () => {
      const publisherData = new Promise((resolve) => {
        CSInterface.addEventListener("com.adobe.events.flash.extension.setstate", function (event: CSEvent) {

          if (event.data === undefined) {
            setPublisherStateData({} as any);
          } else {
            setPublisherStateData(event.data as any);
          }

          resolve(undefined);
        });
      });

      const event = new CSEvent(
        "com.adobe.events.flash.extensionLoaded",
        "APPLICATION",
        undefined,
        "com.scwmake.SupercellSWF.PublishSettings"
      );
      event.data = "SupercellSWF publisher loaded";
      CSInterface.dispatchEvent(event);

      await publisherData;
    };
    getData();

    /* const checkData = async () => {
      const scriptPath =  CSInterface.getSystemPath(SystemPath.HOST_APPLICATION) + require("./JSFL/index.jsfl") ;
      const scriptURI = 'file:///' + 
      scriptPath
        .replace(/\\+/g, '/')
        .replace(/\/+/g, '/')
        .replace(/(^|\/)\.\//img, '$1')
        .replace(/ /g, '%20')
        .replace(/^([a-z ]+):/i, '$1|');

      await CSInterface.evalScript(`fl.runScript("${scriptURI}")`)

      setDataCheck(true);
    }
    checkData(); */

  }, []);

  if (publisherStateData !== undefined && isFontLoaded) {
    State.restore(publisherStateData);

    return createElement(
      "body",
      {
        key: "publisher_body",
        style: {
          fontFamily: "PublisherFont",
          backgroundColor: `#${AppColor.toString(16)}`,
          position: "relative"
        }
      },
      <Publisher></Publisher>
    );
  } else {
    return null;
  }
}

const root = ReactDOM.createRoot(
  document.getElementById('root') as HTMLElement
);

root.render(
  <React.StrictMode>
    <App></App>
  </React.StrictMode>
)
