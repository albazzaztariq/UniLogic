function level1(x) {
    if ((x < 0)) {
        return {ok: false, error: "negative input"};
    }
    return {ok: true, value: (x * 10)};
}

function level2(x) {
    const _r0 = level1(x);
    if (!_r0.ok) {
        return _r0;
    }
    let a = _r0.value;
    return {ok: true, value: (a + 1)};
}

function level3(x) {
    const _r1 = level2(x);
    if (!_r1.ok) {
        return _r1;
    }
    let b = _r1.value;
    return {ok: true, value: (b + 2)};
}

function main() {
    const _r2 = level3(5);
    if (!_r2.ok) {
        console.error("error: " + _r2.error);
        process.exit(1);
    }
    let r1 = _r2.value;
    console.log(r1);
    const _r3 = level3(0);
    if (!_r3.ok) {
        console.error("error: " + _r3.error);
        process.exit(1);
    }
    let r2 = _r3.value;
    console.log(r2);
    const _r4 = level3((-1));
    if (!_r4.ok) {
        console.error("error: " + _r4.error);
        process.exit(1);
    }
    let r3 = _r4.value;
    console.log(r3);
    return 0;
}


main();
