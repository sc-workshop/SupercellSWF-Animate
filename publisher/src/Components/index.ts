
import { getInterface, isCEP } from "../CEP";
import { State } from "./publisherState";

export function publish() {
    console.log(State.data)

    if (!isCEP) {
        return;
    }
    try {
        State.save();
        const CSInterface = getInterface();
        CSInterface.evalScript("fl.getDocumentDOM().publish()", function() {
            CSInterface.closeExtension();
        })
    } catch (error) {
        alert(error);
    }
    
}