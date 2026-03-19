function* mul_table(rows, cols) {
    let r = 1;
    while ((r <= rows)) {
        let c = 1;
        while ((c <= cols)) {
            yield (r * c);
            c += 1;
        }
        r += 1;
    }
}

function* triangle(n) {
    let row = 1;
    while ((row <= n)) {
        let col = 1;
        while ((col <= row)) {
            yield col;
            col += 1;
        }
        row += 1;
    }
}

function main() {
    for (const v of mul_table(2, 3)) {
        console.log(v);
    }
    for (const t of triangle(3)) {
        console.log(t);
    }
    return 0;
}


main();
