
import { getInterface, isCEP } from "../CEP";
import { State } from "./publisherState";

export function publish() {
    if (!isCEP) {
        return;
    }

    State.save();
    const CSInterface = getInterface();
    CSInterface.evalScript("fl.getDocumentDOM().publish()", function() {
        CSInterface.closeExtension();
    })
}