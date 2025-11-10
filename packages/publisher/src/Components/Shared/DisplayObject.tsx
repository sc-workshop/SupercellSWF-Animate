import type { BaseProps, BaseState } from "Components/Shared";
import FloatTip from "Components/Shared/FloatTip";
import TextField from "Components/Shared/TextField";
import { Component, createElement } from "react";

export default class DisplayObject<
	Props extends BaseProps,
	State extends BaseState,
> extends Component<Props, State> {
	public IsAutoProperty = false;

	createLabel() {
		let label = TextField(`${this.props.name} :`, {
			color: "#727776",
			marginRight: "5px",
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

		return label;
	}
}
