
import { getInterface, isCEP } from "CEP";
import { Settings } from "Publisher/PublisherSettings";

export function publish() {
    console.log(Settings.data)

    if (!isCEP) {
        return;
    }

    try {
        Settings.save();
        const CSInterface = getInterface();
        
        (async function(){
            CSInterface.evalScript("fl.getDocumentDOM().publish()");
            CSInterface.closeExtension();
        })()

    } catch (error) {
        alert(error);
    }
    
}