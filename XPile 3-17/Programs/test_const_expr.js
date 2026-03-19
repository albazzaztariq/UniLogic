
const MAX = 100;
const DOUBLE_MAX = (MAX * 2);
const OFFSET = (MAX + 1);
const PI = 3.14159;
const TAU = (PI * 2);
const GREETING = ("Hello" + " World");
const FLAG = true;
function main() {
    console.log(MAX);
    console.log(DOUBLE_MAX);
    console.log(OFFSET);
    console.log(TAU);
    console.log(GREETING);
    console.log(FLAG);
    let x = (MAX + 10);
    console.log(x);
    let y = (DOUBLE_MAX - OFFSET);
    console.log(y);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
