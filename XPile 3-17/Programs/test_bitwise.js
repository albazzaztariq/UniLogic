
function main() {
    let a = 255;
    let b = 15;
    let r1 = (a & b);
    console.log(r1);
    let r2 = (240 | 15);
    console.log(r2);
    let r3 = (255 ^ 15);
    console.log(r3);
    let r4 = (~0);
    console.log(r4);
    let r5 = (1 << 4);
    console.log(r5);
    let r6 = (256 >> 4);
    console.log(r6);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
