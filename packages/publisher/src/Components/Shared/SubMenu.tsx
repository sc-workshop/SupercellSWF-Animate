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
				marginRight: "5px",
			}}
		/>
	);

	const label = TextField(name, {
		color: "white",
		marginRight: "10px",
		fontSize: "18px",
	});

	const header = (
		<div style={{ display: "flex" }}>
			{toggleButton}
			{label}
		</div>
	);

	const delim = createElement("hr", {
		key: `menu_${keyName}_delim`,
		style: {
			width: "50%",
			marginRight: "50%",
			marginTop: "5px",
			border: "2px solid #484848",
		},
	});

	const storage = createElement(
		"div",
		{
			key: `menu_${keyName}_storage`,
			style: {
				paddingLeft: "5%",
				maxHeight: active ? "2000px" : "0px",
				overflow: "hidden",
				transition: "max-height 200ms ease",
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
						transition: `opacity 200ms ease, transform 200ms ease ${index * 30}ms`,
						marginBottom: "5px",
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
			style: style,
		},
		header,
		delim,
		storage,
	);
}
