
function check(val) {
    if ((val < 0)) {
        console.log((-1));
        process.exit(1);
    }
    return (val * 2);
}

function main() {
    let result = check(5);
    console.log(result);
    process.exit(0);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
