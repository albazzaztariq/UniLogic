function minmax(a, b) {
    if ((a < b)) {
        return [a, b];
    }
    return [b, a];
}

function main() {
    let [lo, hi] = minmax(10, 3);
    console.log(lo);
    console.log(hi);
    return 0;
}


main();
