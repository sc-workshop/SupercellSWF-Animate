
// @ts-ignore
Array.prototype.indexOf = Array.prototype.indexOf ? Array.prototype.indexOf :
function (searchElement, fromIndex) {
    var k;

    var O = Object(this);
    var len = O.length >>> 0;
    if (len === 0) {
        return -1;
    }

    var n = +fromIndex || 0;

    if (n >= len) {
        return -1;
    }
    k = Math.max(n >= 0 ? n : len - Math.abs(n), 0);

    while (k < len) {
        if (k in O && O[k] === searchElement) {
            return k;
        }
        k++;
    }
    return -1;
}