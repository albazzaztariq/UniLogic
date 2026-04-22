
function divide(a, b) {
    if ((b === 0)) {
        throw {__ul_type: "DivisionByZero", __ul_msg: "cannot divide by zero", message: "cannot divide by zero"};
    }
    return Math.trunc(a / b);
}

function risky(x) {
    if ((x < 0)) {
        throw {__ul_type: "ValueError", __ul_msg: "negative input", message: "negative input"};
    }
    return (x * 2);
}

function main() {
    console.log(divide(10, 2));
    try {
        let result = divide(10, 0);
        console.log(result);
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        if (__ex_type === "DivisionByZero") {
            let err = __ex_msg;
            console.log("caught: divide by zero");
        } else {
            throw __ex;
        }
    }
    try {
        let val = risky((-5));
        console.log(val);
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        if (__ex_type === "ValueError") {
            let err = __ex_msg;
            console.log("caught: negative input");
        } else {
            throw __ex;
        }
    }
    try {
        let val = risky(10);
        console.log(val);
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        if (__ex_type === "ValueError") {
            let err = __ex_msg;
            console.log("should not reach here");
        } else {
            throw __ex;
        }
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
