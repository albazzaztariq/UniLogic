
function twice(x) {
    return (x * 2);
}

function add_one(x) {
    return (x + 1);
}

function flip_sign(x) {
    return (0 - x);
}

function main() {
    let a = twice(5);
    console.log(a);
    let b = add_one(twice(5));
    console.log(b);
    let c = flip_sign(add_one(twice(3)));
    console.log(c);
    let d = add_one(10);
    console.log(d);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
