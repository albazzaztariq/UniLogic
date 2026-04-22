
function main() {
    let x = 10;
    // @norm 2
    let y = (x + 5);
    // @norm 0
    let z = (y * 2);
    console.log(z);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
