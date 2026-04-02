String.prototype.trim = String.prototype.trim
	? String.prototype.trim
	: function (this: string) {
			return this.replace(/^[\s\uFEFF\xA0]+|[\s\uFEFF\xA0]+$/g, "");
		};
