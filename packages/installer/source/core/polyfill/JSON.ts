// biome-ignore lint/suspicious/noShadowRestrictedNames: Polyfill implementation
// biome-ignore lint/correctness/noUnusedVariables: Polyfill
const JSON = {
	parse: (data: string) => {
		if (!data) {
			return {};
		}

		// @ts-expect-error
		// biome-ignore lint/security/noGlobalEval: this is the only way to parse JSON in older engines
		eval(`var json=${data};`);
		// @ts-expect-error
		return json;
	},
};
