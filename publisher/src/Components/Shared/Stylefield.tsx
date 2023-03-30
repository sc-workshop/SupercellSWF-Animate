import { CSSProperties } from "react";

export function Stylefield(text: string, style: CSSProperties) {
    return <span style={style}>{text}</span>
}