import * as cep from "ts-csinterface"

// * CSInterface
export let CSInterface: cep.CSInterface | undefined = undefined;

try {
  CSInterface = new cep.CSInterface();
} catch (error) {
  console.error("Host is not CEP!");
}


export const CEP = cep;