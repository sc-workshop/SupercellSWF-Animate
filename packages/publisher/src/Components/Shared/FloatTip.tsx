import Locale from "Publisher/Localization";
import {
	autoUpdate,
	flip,
	offset,
	shift,
	useDismiss,
	useFloating,
	useFocus,
	useHover,
	useInteractions,
	useRole,
} from "@floating-ui/react";
import { createElement, useState } from "react";

import { animated, useSpring } from "react-spring";
import wrapText from "wrap-text";
import TextField from "./TextField";

export default function FloatTip(tip_tid: string) {
	const animation_style = useSpring({
		from: {
			opacity: 0,
		},
		to: {
			opacity: 1,
		},
		reset: true,
		config: {
			duration: 100,
		},
	});

	const [isOpen, setIsOpen] = useState(false);

	const { refs, floatingStyles, context } = useFloating({
		open: isOpen,
		onOpenChange: setIsOpen,
		placement: "top-start",
		middleware: [offset(10), flip(), shift()],
		whileElementsMounted: autoUpdate,
	});

	const hover = useHover(context, { move: true, delay: 200 });
	const focus = useFocus(context);
	const dismiss = useDismiss(context);
	const role = useRole(context, {
		role: "tooltip",
	});

	const { getReferenceProps, getFloatingProps } = useInteractions([
		hover,
		focus,
		dismiss,
		role,
	]);

	const tip_text = TextField(
		wrapText(Locale.Get(tip_tid), 50),
		{},
		`${tip_tid}_text`,
	);

	const holder = createElement(
		"div",
		{
			key: tip_tid,
			role: "tooltip",
			style: {
				background: "rgba(25,25,25,255)",
				color: "white",
				fontWeight: "lighter",
				padding: "5px",
				borderRadius: "4px",
				fontSize: "110%",
				width: "max-content",
				whiteSpace: "pre-wrap",
				userSelect: "none",
				MozUserSelect: "none",
				KhtmlUserSelect: "none",
				WebkitUserSelect: "none",
			},
		},
		tip_text,
	);

	return [
		refs.setReference,
		getReferenceProps(),
		isOpen && (
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
		),
	];
}
