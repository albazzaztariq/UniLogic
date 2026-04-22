
function add(a, b) {
    return (a + b);
}

function main() {
    let result = add(10, 20);
    console.log(result);
    console.log(add(3, 4));
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
