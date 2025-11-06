import {
	type CSSProperties,
	createElement,
	type Dispatch,
	type SetStateAction,
} from "react";
import DisplayObject from "./DisplayObject";
import FloatTip from "./FloatTip";
import TextField from "./TextField";

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
		let label = TextField(`${this.props.name} :`, {
			color: "#727776",
			marginRight: "5px",
		});

		if (this.props.tip_tid !== undefined) {
			const [tip_reference, props, tip_element] = FloatTip(this.props.tip_tid);
			label = createElement(
				"div",
				{
					ref: tip_reference,
				},
				label,
				tip_element as never,
				...(props as unknown as never[]),
			);
		}
		const checkbox = createElement("input", {
			key: `boolfield_${this.props.keyName}_input`,
			type: "checkbox",
			onChange: () => {
				this.state.checked = this.state.checked == false;

				if (typeof this.props.callback == "function") {
					this.props.callback(this.state.checked);
				} else {
					this.props.callback[1](this.state.checked);
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
