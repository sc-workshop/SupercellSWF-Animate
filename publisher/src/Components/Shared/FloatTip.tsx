import { ReactElement, createElement, useState } from "react"
import TextField from "./TextField"
import {
    useFloating,
    autoUpdate,
    offset,
    flip,
    shift,
    useHover,
    useFocus,
    useDismiss,
    useRole,
    useInteractions,
    FloatingPortal
} from "@floating-ui/react";
import { useSpring, animated } from "react-spring";
import wrapText = require("wrap-text")
import Locale from "../../Localization"

export default function FloatTip(tip_tid: string) {
    const [isOpen, setIsOpen] = useState(false);

    const { refs, floatingStyles, context } = useFloating({
        open: isOpen,
        onOpenChange: setIsOpen,
        placement: "top-start",

        whileElementsMounted: autoUpdate,
        middleware: [
            offset(5),
            flip({
                fallbackAxisSideDirection: "start"
            }),
            shift()
        ]
    });

    const animation_style = useSpring({
        from: {
            opacity: 0
        },
        to: {
            opacity: 1
        },
        reset: true,
        config: {
            duration: 100
        }
    })

    const hover = useHover(context, { move: false });
    const focus = useFocus(context);
    const dismiss = useDismiss(context);
    const role = useRole(context, { role: "tooltip" });

    const { getReferenceProps, getFloatingProps } = useInteractions([
        hover,
        focus,
        dismiss,
        role
    ]);

    const text = Locale.Get(tip_tid);
    const tip_text = TextField(
        wrapText(text, 50),
        {

        },
        `${tip_tid}_text`
    )

    const holder = createElement(
        "div",
        {
            key: tip_tid,
            role: "tooltip",
            style:
            {
                background: "rgba(25,25,25,255)",
                color: 'white',
                fontWeight: "lighter",
                padding: "5px",
                borderRadius: '4px',
                fontSize: "110%",
                width: "max-content",
                whiteSpace: "pre-wrap"

            }
        },
        tip_text
    )

    return [
        refs.setReference,
        getReferenceProps(),
        <FloatingPortal>
            {isOpen && (
                <animated.div style={animation_style}>
                    <div
                        className="Tooltip"
                        ref={refs.setFloating}
                        style={floatingStyles}
                        {...getFloatingProps()}
                    >
                        {holder}
                    </div>
                </animated.div>
            )}
        </FloatingPortal>
    ]
}