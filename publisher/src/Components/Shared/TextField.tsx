import { CSSProperties, LegacyRef } from "react";

let id = 0;
export default function TextField(text: string, style: CSSProperties, keyName: string = "", reference: any = undefined) {
    const key = keyName == "" ? `txt_${id++}` : keyName;
    return <span key={key} style={style} ref={reference as any}>{text}</span>
}