
function minmax(a, b) {
    if ((a < b)) {
        return [a, b];
    }
    return [b, a];
}

function main() {
    let [lo, hi] = minmax(10, 3);
    console.log(lo);
    console.log(hi);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
