
function main() {
    let a = 42;
    let b = Number(a);
    console.log(b);
    let c = 3.7;
    let d = Math.trunc(c);
    console.log(d);
    let e = 123;
    let f = String(e);
    console.log(f);
    let g = true;
    let h = Math.trunc(g);
    console.log(h);
    let i = 0;
    let j = Boolean(i);
    console.log(j);
    let k = 1;
    let l = Boolean(k);
    console.log(l);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
