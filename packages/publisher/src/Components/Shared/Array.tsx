/** biome-ignore-all lint/a11y/useKeyWithClickEvents: no sense of keyboard */
import type { BaseProps } from "Components/Shared";
import IconButton from "Components/Shared/Button";
import TextField from "Components/Shared/TextField";
import {
	Component,
	type ReactNode,
	useLayoutEffect,
	useMemo,
	useRef,
	useState,
} from "react";

type Props<T, BaseT> = BaseProps & {
	onCreate(id: number, value?: BaseT): T;
	onRemove?: (id: number) => void;
	onChange?: (elements: T[]) => void;
	initValues?: BaseT[];
};

type AnimatedElement<T> = {
	value: T;
	id: number;
	state: "entering" | "normal" | "exiting";
};

export default function ElementsArray<T extends ReactNode | Component, BaseT>(
	props: Props<T, BaseT>,
) {
	const [elements, setElements] = useState<AnimatedElement<T>[]>([]);
	const [selected, setSelected] = useState<number | null>(null);
	const [active, setActive] = useState(false);
	const [height, setHeight] = useState("40px");

	const containerRef = useRef<HTMLDivElement>(null);
	const idCounter = useRef(0);
	const deleteTimeout = useRef<NodeJS.Timeout | null>(null);

	const initialize = () => {
		if (elements.length !== 0) return;

		if (props.initValues && props.initValues.length > 0) {
			const initialized = props.initValues.map((value) => {
				const id = idCounter.current++;
				return {
					value: props.onCreate(id, value),
					id,
					state: "normal" as const,
				};
			});
			setElements(initialized);
		}
	};

	// biome-ignore lint/correctness/useExhaustiveDependencies: external function
	useMemo(initialize, [props.initValues]);

	const handleAdd = () => {
		const id = idCounter.current++;
		const newElement: AnimatedElement<T> = {
			value: props.onCreate(id),
			id: id,
			state: "entering",
		};
		const newElements = [...elements, newElement];
		setElements(newElements);

		if (props.onChange) props.onChange(newElements.map((e) => e.value));

		setTimeout(() => {
			setElements((prev) =>
				prev.map((e) =>
					e.id === newElement.id ? { ...e, state: "normal" } : e,
				),
			);
		}, 20);
	};

	const handleDelete = () => {
		if (elements.length === 0) return;

		if (deleteTimeout.current) {
			clearTimeout(deleteTimeout.current);
			deleteTimeout.current = null;
		}

		const targets = selected !== null ? [selected] : [elements.length - 1];
		const alreadyExiting = targets.some(
			(i) => elements[i]?.state === "exiting",
		);

		if (alreadyExiting) {
			const filtered = elements.filter((_, i) => !targets.includes(i));
			setElements(filtered);
			if (props.onChange) props.onChange(filtered.map((e) => e.value));
			setSelected(null);
			return;
		}

		setElements((prev) =>
			prev.map((e, i) =>
				targets.includes(i) ? { ...e, state: "exiting" } : e,
			),
		);

		setSelected(null);
		deleteTimeout.current = setTimeout(() => {
			if (props.onRemove) {
				for (const index of targets) {
					props.onRemove(elements[index].id);
				}
			}

			const filtered = elements.filter((_, i) => !targets.includes(i));
			setElements(filtered);

			if (props.onChange) props.onChange(filtered.map((e) => e.value));

			deleteTimeout.current = null;
		}, 200);
	};

	const selectElement = (index: number) => {
		setSelected(index === selected ? null : index);
	};

	// biome-ignore lint/correctness/useExhaustiveDependencies: elements adding
	useLayoutEffect(() => {
		if (containerRef.current) {
			const scrollHeight = containerRef.current.scrollHeight;
			setHeight(active ? `${scrollHeight}px` : "40px");
		}
	}, [active, elements]);

	const getAnimatedStyle = (
		e: AnimatedElement<T>,
		_index: number,
		isSelected: boolean,
	): React.CSSProperties => {
		const base: React.CSSProperties = {
			marginBottom: "4px",
			padding: "6px",
			borderRadius: "4px",
			cursor: "pointer",
			backgroundColor: isSelected ? "#4e8dd4" : "#2e2e2e",
			transition: "all 180ms ease",
			opacity: 1,
			transform: "translateY(0)",
		};

		if (e.state === "entering") {
			base.opacity = 0;
			base.transform = "translateY(-6px)";
		} else if (e.state === "exiting") {
			base.opacity = 0;
			base.transform = "translateY(-6px)";
		}

		return base;
	};

	const toggleButton = (
		<IconButton
			src={require(`../../Assets/images/arrow.png`)}
			alt="Toggle"
			onClick={() => setActive(!active)}
			rotate
			active={active}
		/>
	);
	const addButton = (
		<IconButton
			src={require(`../../Assets/images/plus.png`)}
			alt="Add"
			onClick={handleAdd}
		/>
	);

	const deleteButton = (
		<IconButton
			src={require(`../../Assets/images/minus.png`)}
			alt="Delete"
			onClick={handleDelete}
		/>
	);

	const header = (
		<div
			style={{
				display: "flex",
				alignItems: "center",
				justifyContent: "space-between",
				height: "34px",
				backgroundColor: "#2a2a2a",
				marginBottom: "6px",
				padding: "0 6px",
				borderBottom: "1px solid #1d1d1d",
			}}
		>
			{toggleButton}
			<div style={{ display: "flex", gap: "6px" }}>
				{addButton}
				{deleteButton}
			</div>
		</div>
	);

	const contentContainer = (
		<div style={{ position: "relative", width: "98%" }}>
			<div
				ref={containerRef}
				style={{
					height,
					overflow: "hidden",
					transition: "height 250ms ease",
					position: "relative",
					backgroundColor: "#1f1f1f",
				}}
			>
				<ul style={{ padding: 0, margin: 0, listStyle: "none" }}>
					{elements.map((e, index) => {
						const isSelected = selected === index;
						let value: ReactNode | Component = e.value;
						if (value instanceof Component) value = value.render();

						return (
							<li
								key={e.id}
								onClick={() => selectElement(index)}
								style={getAnimatedStyle(e, index, isSelected)}
							>
								{value}
							</li>
						);
					})}
				</ul>

				{!active && elements.length > 0 && (
					<div
						style={{
							position: "absolute",
							bottom: 0,
							left: 0,
							right: 0,
							height: "40%",
							background:
								"linear-gradient(to bottom, rgba(31,31,31,0) 0%, rgba(0,0,0,0.6) 100%)",
							pointerEvents: "none",
							transition: "opacity 200ms",
						}}
					/>
				)}
			</div>
		</div>
	);

	const containerWrapper = (
		<div
			style={{
				width: "50%",
				backgroundColor: "#222",
				borderRadius: "6px",
				overflow: "hidden",
				position: "relative",
				padding: "4px",
				boxShadow: "inset 0 0 4px rgba(0,0,0,0.4)",
				...props.style,
			}}
		>
			{header}
			{contentContainer}
		</div>
	);

	const elementsCounter = TextField(`+${elements.length - 1}`, {
		color: "#ccc",
		marginLeft: "5px",
		marginTop: "-15px",
		position: "absolute",
		fontSize: "11px",
	});

	return (
		<div>
			{containerWrapper}
			{elements.length > 1 && !active ? elementsCounter : ""}
		</div>
	);
}
