import type React from "react";
import {
	type CSSProperties,
	createElement,
	type Dispatch,
	type ReactNode,
	type SetStateAction,
} from "react";
import DisplayObject from "./DisplayObject";
import FloatTip from "./FloatTip";
import TextField from "./TextField";

type Enum = {
	[id: number]: unknown;
	[id: string]: unknown;
};

type EnumArray = string[] | number[];

type callback = (value: string) => void;
type state = [string, Dispatch<SetStateAction<string>>];

type Props = {
	name: string;
	keyName: string;
	enumeration: Enum | EnumArray;
	defaultValue: string | number;
	style: CSSProperties;
	callback: callback | state;
	tip_tid?: string;
};

type State = {
	focus: boolean;
	value: unknown;
	options: ReactNode[];
};

export default class EnumField extends DisplayObject<Props, State> {
	public static readonly defaultProps = {
		tip_tid: undefined,
	};

	state: State = {
		focus: false,
		value: undefined,
		options: [],
	};

	constructor(props: Props) {
		super(props);

		this.state.value = this.props.defaultValue;

		let maxEnumSize = 0;
		if (Array.isArray(this.props.enumeration)) {
			for (let i = 0; this.props.enumeration.length > i; i++) {
				const key = String(this.props.enumeration[i]);

				if (key.length > maxEnumSize) {
					maxEnumSize = key.length;
				}

				this.state.options.push(
					<option key={`enumfield_${this.props.keyName}_${key}`} value={i}>
						{key}
					</option>,
				);
			}
		} else {
			const enumKeys = Object.keys(this.props.enumeration);
			const delimIndex = enumKeys.length / 2;

			for (let i = 0; delimIndex > i; i++) {
				const enumKey = enumKeys[i + delimIndex];
				const enumValue = enumKeys[i];

				const keyStr = String(enumKey);
				if (keyStr.length > maxEnumSize) {
					maxEnumSize = keyStr.length;
				}

				this.state.options.push(
					<option
						key={`enumfield_${this.props.keyName}_${enumValue}`}
						value={enumValue}
					>
						{enumKey}
					</option>,
				);
			}
		}
	}

	render() {
		let label = TextField(`${this.props.name} :`, {
			color: "#727776",
		});

		if (this.props.tip_tid !== undefined) {
			const [tip_reference, , tip_element] = FloatTip(this.props.tip_tid);
			label = createElement(
				"div",
				{
					ref: tip_reference,
				},
				label,
				tip_element as never,
			);
		}
		const selector = createElement(
			"select",
			{
				key: `enumfield_${this.props.keyName}_input`,
				style: {
					width: `100px !important`,
					height: "35px",
					color: "white",
					marginLeft: "5px",
					background: "#1a1a1a",
					borderRadius: "5px",
					outline: this.state.focus ? "2px solid #337ed4" : "none",
					border: "2px solid #363636",
				},
				defaultValue: this.props.defaultValue,
				onChange: (event: React.FormEvent<HTMLInputElement>) => {
					if (typeof this.props.callback == "function") {
						this.props.callback(event.currentTarget.value);
					} else {
						this.props.callback[1](event.currentTarget.value);
					}

					this.state.value = event.currentTarget.value;
				},
				onFocus: () => {
					this.state.focus = true;
				},
				onBlur: () => {
					this.state.focus = false;
				},
			},
			this.state.options,
		);

		return createElement(
			"div",
			{
				key: `enumfield_${this.props.keyName}`,
				style: this.props.style,
			},
			label,
			selector,
		);
	}
}
