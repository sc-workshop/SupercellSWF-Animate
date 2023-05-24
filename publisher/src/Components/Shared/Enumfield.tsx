import { CSSProperties, createElement } from "react";

interface Enum {
    [id: number]: any;
    [id: string]: any;
}

type EnumArray = string[] | number[];

export function Enumfield(enumeration: Enum | EnumArray, defaultValue: string, selectorStyle: CSSProperties, onChange: (value: string) => void) {
    const options = [];

    if (Array.isArray(enumeration)) {
        for (const key of enumeration) {
            options.push(createElement("option", {
    
                value: key,
                style: {
    
                }
            }, key))
        }
    } else {
        for (let i = 0; Object.keys(enumeration).length / 2 > i; i++) {
            const enumKey = enumeration[i];
    
            options.push(createElement("option", {
    
                value: enumKey,
                style: {
    
                }
            }, enumKey))
        }
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