import type { CSSProperties } from "react";

export type BaseProps = {
	name?: string;
	keyName: string;
	style?: CSSProperties;
	tip_tid?: string;
};

export type BaseState = {
	focus: boolean;
};
