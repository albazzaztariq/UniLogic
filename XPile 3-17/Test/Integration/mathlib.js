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


main();
