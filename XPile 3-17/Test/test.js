function add(a, b) {
    return (a + b);
}

function factorial(n) {
    if ((n <= 1)) {
        return 1;
    }
    return (n * factorial((n - 1)));
}

function is_even(n) {
    return ((n % 2) === 0);
}

function clamp(val, lo, hi) {
    if ((val < lo)) {
        return lo;
    }
    if ((val > hi)) {
        return hi;
    }
    return val;
}

function main() {
    let x = add(3, 7);
    console.log(x);
    const limit = 100;
    let i = 0;
    while ((i < 5)) {
        console.log(i);
        i += 1;
    }
    if (is_even(x)) {
        console.log(1);
    } else {
        console.log(0);
    }
    let ratio = Number(x);
    console.log(ratio);
    let clamped = clamp(x, 0, 8);
    console.log(clamped);
    let fact = factorial(5);
    console.log(fact);
    let nums = [10, 20, 30, 40, 50];
    for (const n of nums) {
        console.log(n);
    }
    return 0;
}


main();
