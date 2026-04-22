
function factorial(n) {
    if ((n <= 1)) {
        return 1;
    }
    return (n * factorial((n - 1)));
}

function square(x) {
    return (x * x);
}

function add(a, b) {
    return (a + b);
}

function main() {
    let f = factorial(5);
    console.log(f);
    let s = square(7);
    console.log(s);
    let sum = add(10, 20);
    console.log(sum);
    let composed = square(add(3, 4));
    console.log(composed);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
