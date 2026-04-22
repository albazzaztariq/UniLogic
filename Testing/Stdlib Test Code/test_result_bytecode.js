
function divide(a, b) {
    if ((b === 0)) {
        return {ok: false, error: "division by zero"};
    }
    return {ok: true, value: Math.trunc(a / b)};
}

function main() {
    const _r0 = divide(10, 2);
    if (!_r0.ok) {
        console.error("error: " + _r0.error);
        process.exit(1);
    }
    let r1 = _r0.value;
    console.log(r1);
    const _r1 = divide(100, 5);
    if (!_r1.ok) {
        console.error("error: " + _r1.error);
        process.exit(1);
    }
    let r2 = _r1.value;
    console.log(r2);
    const _r2 = divide(42, 7);
    if (!_r2.ok) {
        console.error("error: " + _r2.error);
        process.exit(1);
    }
    let r3 = _r2.value;
    console.log(r3);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
