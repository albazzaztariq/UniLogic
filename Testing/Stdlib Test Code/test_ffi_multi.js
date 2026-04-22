const fmod = (x, y) => x % y;
const pow = Math.pow;
const sqrt = Math.sqrt;


function main() {
    let s = sqrt(625.0);
    console.log(s);
    let p = pow(2.0, 10.0);
    console.log(p);
    let m = fmod(17.0, 5.0);
    console.log(m);
    let chained = sqrt(pow(3.0, 4.0));
    console.log(chained);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
