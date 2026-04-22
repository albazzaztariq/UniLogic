const ceil = Math.ceil;
const floor = Math.floor;
const pow = Math.pow;
const sqrt = Math.sqrt;

function math_sqrt(x) {
    return sqrt(x);
}

function math_pow(base, exp) {
    return pow(base, exp);
}

function math_abs(x) {
    if ((x < 0.0)) {
        return (0.0 - x);
    }
    return x;
}

function math_floor(x) {
    return floor(x);
}

function math_ceil(x) {
    return ceil(x);
}

function math_min(a, b) {
    if ((a < b)) {
        return a;
    }
    return b;
}

function math_max(a, b) {
    if ((a > b)) {
        return a;
    }
    return b;
}

function math_pi() {
    return 3.141592653589793;
}

function math_e() {
    return 2.718281828459045;
}

function main() {
    let s = math_sqrt(144.0);
    console.log(s);
    let p = math_pow(2.0, 10.0);
    console.log(p);
    let a = math_abs((0.0 - 42.5));
    console.log(a);
    let fl = math_floor(3.7);
    console.log(fl);
    let ce = math_ceil(3.2);
    console.log(ce);
    let mn = math_min(5.0, 3.0);
    console.log(mn);
    let mx = math_max(5.0, 3.0);
    console.log(mx);
    let pi = math_pi();
    console.log(pi);
    let e = math_e();
    console.log(e);
    return 0;
}


main();
