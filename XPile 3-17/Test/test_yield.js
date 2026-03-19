function* countdown(n) {
    while ((n > 0)) {
        yield n;
        n -= 1;
    }
}

function* doubles(n) {
    let i = 1;
    while ((i <= n)) {
        yield (i * 2);
        i += 1;
    }
}

function main() {
    for (const val of countdown(5)) {
        console.log(val);
    }
    for (const d of doubles(4)) {
        console.log(d);
    }
}


main();
