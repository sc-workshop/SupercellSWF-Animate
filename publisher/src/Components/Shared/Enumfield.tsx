import { CSSProperties, createElement } from "react";

interface Enum {
    [id: number]: any;
    [id: string]: any;
}

export function Enumfield(enumeration: Enum, defaultValue: string, selectorStyle: CSSProperties, onChange: (value: string) => void) {
    const options = [];

    for (let i = 0; Object.keys(enumeration).length / 2 > i; i++) {
        const enumKey = enumeration[i];

        options.push(createElement("option", {

            value: enumKey,
            style: {

            }
        }, enumKey))
    }

    return createElement("select", {
        style: selectorStyle,
        defaultValue: defaultValue,
        onChange: function (event: React.FormEvent<HTMLInputElement>) {
            onChange(event.currentTarget.value)
        }
    }, options);
    ;
}