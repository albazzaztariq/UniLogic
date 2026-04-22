
function main() {
    let a = ((5) * 2);
    console.log(a);
    let b = ((3) + (7));
    console.log(b);
    let c = ((6) * (6));
    console.log(c);
    let d = ((((2) * 2)) + (((3) * (3))));
    console.log(d);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
