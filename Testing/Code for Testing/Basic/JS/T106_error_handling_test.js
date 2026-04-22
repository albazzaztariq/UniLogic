
function safe_divide(a, b) {
    if ((b === 0)) {
        return {ok: false, error: "division by zero"};
    }
    return {ok: true, value: Math.trunc(a / b)};
}

function chain_calc(x) {
    const _r0 = safe_divide(x, 2);
    if (!_r0.ok) {
        return _r0;
    }
    let half = _r0.value;
    let doubled = (half * 2);
    return {ok: true, value: doubled};
}

function risky_divide(a, b) {
    if ((b === 0)) {
        throw {__ul_type: "ArithmeticError", __ul_msg: "cannot divide by zero", message: "cannot divide by zero"};
    }
    return Math.trunc(a / b);
}

function main() {
    try {
        let r = risky_divide(10, 0);
        console.log("should not reach");
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        if (__ex_type === "ArithmeticError") {
            let e = __ex_msg;
            console.log(("caught: " + e));
        } else {
            throw __ex;
        }
    }
    try {
        let r = risky_divide(10, 2);
        console.log(r);
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        if (__ex_type === "ArithmeticError") {
            console.log("should not catch");
        } else {
            throw __ex;
        }
    }
    try {
        console.log("in try");
    } finally {
        console.log("in finally");
    }
    try {
        throw {__ul_type: "SomeError", __ul_msg: "unknown problem", message: "unknown problem"};
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        {
            let e = __ex_msg;
            console.log(("catch-all: " + e));
        }
    }
    try {
        throw {__ul_type: "TestError", __ul_msg: "test", message: "test"};
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        if (__ex_type === "TestError") {
            let e = __ex_msg;
            console.log(("caught: " + e));
        } else {
            throw __ex;
        }
    } finally {
        console.log("finally ran");
    }
    const _r1 = safe_divide(10, 2);
    if (!_r1.ok) {
        console.error("error: " + _r1.error);
        process.exit(1);
    }
    let a = _r1.value;
    console.log(a);
    const _r2 = chain_calc(20);
    if (!_r2.ok) {
        console.error("error: " + _r2.error);
        process.exit(1);
    }
    let b = _r2.value;
    console.log(b);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
