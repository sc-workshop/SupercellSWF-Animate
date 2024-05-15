
const JSON = {
    parse: function (data: string) {
        if (!data) {
            return {};
        }

        // @ts-ignore
        eval("var json=" + data + ";");
        // @ts-ignore
        return json;
    }
}