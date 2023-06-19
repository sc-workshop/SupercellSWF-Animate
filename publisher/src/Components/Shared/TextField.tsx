import { CSSProperties } from "react";

let id = 0;
export default function TextField(text: string, style: CSSProperties, keyName: string = "") {
    const key = keyName == "" ? `txt_${id++}` : keyName;
    return <span key={key} style={style}>{text}</span>
}