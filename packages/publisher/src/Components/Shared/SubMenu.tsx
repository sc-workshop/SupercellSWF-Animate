/** biome-ignore-all lint/a11y/noStaticElementInteractions: no sense of keyboard  */
/** biome-ignore-all lint/a11y/useKeyWithClickEvents: same  */
import IconButton from "Components/Shared/Button";
import {
	type CSSProperties,
	createElement,
	type ReactNode,
	useState,
} from "react";
import TextField from "./TextField";

export default function SubMenu(
	name: string,
	keyName: string,
	style: CSSProperties = {},
	...items: ReactNode[]
) {
	const [active, setActive] = useState(false);

	const toggleButton = (
		<IconButton
			src={require(`../../Assets/images/arrow.png`)}
			alt="Toggle"
			onClick={() => setActive(!active)}
			rotate
			active={active}
			size={24}
			style={{
				verticalAlign: "middle",
				cursor: "pointer",
				marginRight: "8px",
				transition: "transform 0.2s ease, filter 0.2s ease",
				filter: active ? "brightness(1.2)" : "brightness(1)",
				marginLeft: "4px",
			}}
		/>
	);

	const label = TextField(name, {
		color: "white",
		marginRight: "10px",
		fontSize: "18px",
		fontWeight: 500,
		transition: "color 0.2s ease",
	});

	const header = (
		<div
			style={{
				display: "flex",
				alignItems: "center",
				padding: "6px 0",
				cursor: "pointer",
				transition: "all 0.2s ease",
				borderRadius: "8px",
			}}
			onClick={() => setActive(!active)}
			onMouseEnter={(e) => {
				e.currentTarget.style.backgroundColor = "rgba(255, 255, 255, 0.05)";
			}}
			onMouseLeave={(e) => {
				e.currentTarget.style.backgroundColor = "transparent";
			}}
		>
			{toggleButton}
			{label}
		</div>
	);

	const delim = createElement("hr", {
		key: `menu_${keyName}_delim`,
		style: {
			width: "60%",
			marginRight: "40%",
			marginTop: "6px",
			marginBottom: "6px",
			border: "none",
			height: "1px",
			background: "linear-gradient(to right, #484848, transparent)",
		},
	});

	const storage = createElement(
		"div",
		{
			key: `menu_${keyName}_storage`,
			style: {
				paddingLeft: "8%",
				maxHeight: active ? "2000px" : "0px",
				overflow: "hidden",
				transition: "max-height 0.3s ease-in-out",
			},
		},
		...items.map((value, index) =>
			createElement(
				"div",
				{
					key: `submenu_element_${index}_${keyName}`,
					style: {
						opacity: active ? 1 : 0,
						transform: active ? "translateY(0)" : "translateY(-5px)",
						transition: `opacity 0.3s ease, transform 0.3s ease ${index * 40}ms`,
						marginBottom: "10px",
					},
				},
				value,
			),
		),
	);

	return createElement(
		"div",
		{
			key: `menu_${keyName}`,
			style: {
				...style,
				transition: "background-color 0.2s ease",
			},
		},
		header,
		delim,
		storage,
	);
}
