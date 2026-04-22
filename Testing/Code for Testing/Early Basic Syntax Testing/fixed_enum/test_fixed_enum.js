const Priority = Object.freeze({
    low: 1, medium: 2, high: 3,
    _names: {[1]: "low", [2]: "medium", [3]: "high"},
    _values: new Set([1, 2, 3]),
    name(val) { return this._names[val] || "unknown"; },
    valid(val) { return this._values.has(val); }
});

function main() {
    let p = Priority.high;
    console.log(p);
    let q = Priority.low;
    console.log(q);
    if ((p > q)) {
        console.log("high > low");
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
