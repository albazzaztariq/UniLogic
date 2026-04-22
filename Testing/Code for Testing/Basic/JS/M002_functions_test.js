
function add(a, b) {
    return (a + b);
}

function multiply(a, b) {
    return (a * b);
}

function factorial(n) {
    if ((n <= 1)) {
        return 1;
    }
    return (n * factorial((n - 1)));
}

function greet(name) {
    console.log(("Hello, " + name));
}

function main() {
    console.log(add(3, 7));
    console.log(multiply(6, 8));
    console.log(factorial(5));
    console.log(factorial(10));
    greet("Tariq");
    console.log(add(multiply(3, 4), 5));
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
