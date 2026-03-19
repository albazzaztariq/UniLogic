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


main();
