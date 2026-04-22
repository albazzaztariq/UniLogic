
function* countdown(n) {
    while ((n > 0)) {
        yield n;
        n -= 1;
    }
}

function* doubles(n) {
    let i = 1;
    while ((i <= n)) {
        yield (i * 2);
        i += 1;
    }
}

function main() {
    for (const val of countdown(5)) {
        console.log(val);
    }
    for (const d of doubles(4)) {
        console.log(d);
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
