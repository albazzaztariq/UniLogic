
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

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
