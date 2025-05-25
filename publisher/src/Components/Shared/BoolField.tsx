import { CSSProperties, Dispatch, SetStateAction, createElement } from "react";
import TextField from "./TextField";
import FloatTip from "./FloatTip";
import DisplayObject from "./DisplayObject";

type callback = (value: boolean) => void;
type state = [boolean, Dispatch<SetStateAction<boolean>>];

type Props = {
    name: string
    keyName: string
    defaultValue: boolean
    style: CSSProperties
    callback: callback | state,
    tip_tid?: string
}

type State = {
    focus: boolean
    checked: boolean
}

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
        let label = TextField(
            `${this.props.name} :`,
            {
                color: "#727776",
                marginRight: "5px"
            }
        );

        if (this.props.tip_tid !== undefined)
        {
            const [tip_reference, props, tip_element] = FloatTip(this.props.tip_tid);
            label = createElement(
                "div",
                {
                    ref: tip_reference
                },
                label,
                tip_element as never,
                ...props as unknown as never[]
            );
        }

        // eslint-disable-next-line @typescript-eslint/no-this-alias
        const current_element = this;
        const checkbox = createElement(
            "input",
            {
                key: `boolfield_${this.props.keyName}_input`,
                type: "checkbox",
                onChange: function () {
                    current_element.state.checked = current_element.state.checked == false;

                    if (typeof current_element.props.callback == "function") {
                        current_element.props.callback(current_element.state.checked);
                    } else {
                        current_element.props.callback[1](current_element.state.checked);
                    }
                },
                style: {
                    width: `15px`,
                    height: "15px",
                    accentColor: "black",
                    color: "black",
                    backgroundColor: "black",
                    borderRadius: "30px",
                    outline: current_element.state.focus ? "2px solid #337ed4" : "none",
                    border: "2px solid #363636",
                },
                defaultValue: current_element.props.defaultValue,
                defaultChecked: current_element.props.defaultValue,
                onFocus: function () {
                    current_element.state.focus = true;
                },
                onBlur: function () {
                    current_element.state.focus = false;
                }
            }
        );
            
        return createElement(
            "div",
            {
                key: `boolfield_${current_element.props.keyName}`,
                style: current_element.props.style
            },
            label,
            checkbox,
        );
    }
}