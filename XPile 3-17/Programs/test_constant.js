
const MAX = 100;
const VER = "1.0";
function main() {
    console.log(MAX);
    console.log(VER);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
