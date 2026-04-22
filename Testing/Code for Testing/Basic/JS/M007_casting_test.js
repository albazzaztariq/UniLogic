
function main() {
    let f = 3.9;
    let n = Math.trunc(f);
    console.log(n);
    let s = String(42);
    console.log(s);
    let g = Number(10);
    console.log(g);
    let b1 = Boolean(1);
    console.log(b1);
    let b2 = Boolean(0);
    console.log(b2);
    let score = 100;
    let msg = ("Score: " + String(score));
    console.log(msg);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
