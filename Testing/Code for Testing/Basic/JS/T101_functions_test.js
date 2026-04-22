
function add(a, b) {
    return (a + b);
}

function greet(who) {
    return ("Hello " + who);
}

function square(n) {
    return (n * n);
}

function is_positive(n) {
    if ((n > 0)) {
        return true;
    }
    return false;
}

function factorial(n) {
    if ((n <= 1)) {
        return 1;
    }
    return (n * factorial((n - 1)));
}

function no_return() {
    console.log("side effect");
}

function main() {
    console.log(add(3, 7));
    console.log(greet("World"));
    console.log(square(6));
    console.log(is_positive(5));
    console.log(is_positive((-3)));
    console.log(factorial(5));
    no_return();
    console.log(add(square(2), square(3)));
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
