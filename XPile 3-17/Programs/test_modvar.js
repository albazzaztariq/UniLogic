
let MAX_SIZE = 1024;
let PI = 3.14159;
let GREETING = "hello";
let PRIMES = [2, 3, 5, 7, 11];
function double_max() {
    return (MAX_SIZE * 2);
}

function main() {
    console.log(MAX_SIZE);
    console.log(PI);
    console.log(GREETING);
    console.log(PRIMES[0]);
    console.log(PRIMES[4]);
    console.log(double_max());
    MAX_SIZE = 2048;
    console.log(MAX_SIZE);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
