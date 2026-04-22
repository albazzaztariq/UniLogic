
function add(a, b) {
    return (a + b);
}

function main() {
    let x = add(10, 32);
    console.log(x);
    let y = (x * 2);
    console.log(y);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
