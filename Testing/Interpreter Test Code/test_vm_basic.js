
function main() {
    console.log(42);
    console.log(10);
    let x = 5;
    let y = 10;
    console.log((x + y));
    console.log((x * y));
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
