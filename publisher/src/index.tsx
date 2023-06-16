import ReactDOM from 'react-dom/client';
import Publisher from './Publisher';
import React, { useState, createElement, useEffect } from 'react';
import { AppColor } from './Components/themes';
import { getInterface, CSEvent, isCEP } from './CEP';
import { State } from './Components/publisherState';

function App() {
  const [publisherStateData, setPublisherStateData] = useState(undefined);

  useEffect(() => {
    if (!isCEP()){setPublisherStateData({} as any); return;};

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
  }, []);

  if (publisherStateData !== undefined) {
    State.restore(publisherStateData);
    
    return createElement("body",
      {
        style: {
          backgroundColor: `#${AppColor.toString(16)}`
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
  <App></App>
)
