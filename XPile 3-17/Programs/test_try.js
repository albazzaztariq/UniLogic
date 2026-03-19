
function divide(a, b) {
    if ((b === 0)) {
        throw {__ul_type: "ZeroDivisionError", __ul_msg: "division by zero", message: "division by zero"};
    }
    return Math.trunc(a / b);
}

function main() {
    try {
        let result = divide(10, 0);
        console.log("should not reach here");
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        if (__ex_type === "ZeroDivisionError") {
            console.log("test1: caught division by zero");
        } else {
            throw __ex;
        }
    }
    try {
        throw {__ul_type: "ValueError", __ul_msg: "bad input", message: "bad input"};
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        if (__ex_type === "ValueError") {
            let e = __ex_msg;
            console.log(("test2: " + e));
        } else {
            throw __ex;
        }
    }
    try {
        console.log("test3: in try");
    } finally {
        console.log("test3: in finally");
    }
    try {
        throw {__ul_type: "RuntimeError", __ul_msg: "something went wrong", message: "something went wrong"};
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        {
            let e = __ex_msg;
            console.log(("test4: " + e));
        }
    }
    try {
        throw {__ul_type: "TypeError", __ul_msg: "type mismatch", message: "type mismatch"};
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        if (__ex_type === "ValueError") {
            console.log("test5: not this one");
        } else if (__ex_type === "TypeError") {
            let e = __ex_msg;
            console.log(("test5: " + e));
        } else {
            throw __ex;
        }
    }
    try {
        throw {__ul_type: "TestError", __ul_msg: "oops", message: "oops"};
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        if (__ex_type === "TestError") {
            console.log("test6: caught");
        } else {
            throw __ex;
        }
    } finally {
        console.log("test6: finally ran");
    }
    try {
        let x = divide(10, 2);
        console.log(("test7: " + String(x)));
    } catch (__ex) {
        let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");
        let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : "");
        if (__ex_type === "ZeroDivisionError") {
            console.log("test7: should not reach");
        } else {
            throw __ex;
        }
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
