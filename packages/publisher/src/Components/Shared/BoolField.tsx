import {
	type CSSProperties,
	createElement,
	type Dispatch,
	type SetStateAction,
} from "react";
import DisplayObject from "./DisplayObject";

type callback = (value: boolean) => void;
type state = [boolean, Dispatch<SetStateAction<boolean>>];

type Props = {
	name: string;
	keyName: string;
	defaultValue: boolean;
	style: CSSProperties;
	callback: callback | state;
	tip_tid?: string;
};

type State = {
	focus: boolean;
	checked: boolean;
};

export default class BoolField extends DisplayObject<Props, State> {
	public static readonly defaultProps = {
		tip_tid: undefined,
	};

	state: State = {
		focus: false,
		checked: false,
	};

	constructor(props: Props) {
		super(props);

		this.state.checked = props.defaultValue;
	}

	render() {
		const label = this.createLabel();

		const checkbox = createElement("input", {
			key: `boolfield_${this.props.keyName}_input`,
			type: "checkbox",
			onChange: () => {
				const value = this.state.checked == false;
				this.state.checked = value;

				if (typeof this.props.callback == "function") {
					this.props.callback(value);
				} else {
					this.props.callback[1](value);
				}
			},
			style: {
				width: `15px`,
				height: "15px",
				accentColor: "black",
				color: "black",
				backgroundColor: "black",
				borderRadius: "30px",
				outline: this.state.focus ? "2px solid #337ed4" : "none",
				border: "2px solid #363636",
				boxShadow: "none",
			},
			defaultValue: this.props.defaultValue,
			defaultChecked: this.props.defaultValue,
			onFocus: () => {
				this.state.focus = true;
			},
			onBlur: () => {
				this.state.focus = false;
			},
		});

		return createElement(
			"div",
			{
				key: `boolfield_${this.props.keyName}`,
				style: this.props.style,
			},
			label,
			checkbox,
		);
	}
}
