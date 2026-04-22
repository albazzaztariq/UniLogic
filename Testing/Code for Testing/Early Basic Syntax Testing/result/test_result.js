
function divide(a, b) {
    if ((b === 0)) {
        return {ok: false, error: "division by zero"};
    }
    return {ok: true, value: Math.trunc(a / b)};
}

function safe_calc(x, y) {
    const _r0 = divide(x, y);
    if (!_r0.ok) {
        return _r0;
    }
    let result = _r0.value;
    return {ok: true, value: (result * 2)};
}

function main() {
    const _r1 = divide(10, 2);
    if (!_r1.ok) {
        console.error("error: " + _r1.error);
        process.exit(1);
    }
    let a = _r1.value;
    console.log(a);
    const _r2 = safe_calc(20, 4);
    if (!_r2.ok) {
        console.error("error: " + _r2.error);
        process.exit(1);
    }
    let b = _r2.value;
    console.log(b);
    const _r3 = divide(5, 0);
    if (!_r3.ok) {
        console.error("error: " + _r3.error);
        process.exit(1);
    }
    let c = _r3.value;
    console.log(c);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
