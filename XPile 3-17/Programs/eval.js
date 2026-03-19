function str_char_at(s, i) { return i >= 0 && i < s.length ? s[i] : ''; }
function str_substr(s, start, len) { return s.substr(start, len); }
function strlen(s) { return s.length; }


function skip_spaces(expr, pos) {
    let len = strlen(expr);
    while ((pos[0] < len)) {
        let ch = str_char_at(expr, pos[0]);
        if ((ch === " ")) {
            pos[0] = (pos[0] + 1);
        } else {
            break;
        }
    }
}

function parse_number(expr, pos) {
    let len = strlen(expr);
    skip_spaces(expr, pos);
    if ((pos[0] >= len)) {
        return {ok: false, error: "unexpected end of expression"};
    }
    let start = pos[0];
    let negative = false;
    let ch = str_char_at(expr, pos[0]);
    if ((ch === "-")) {
        negative = true;
        pos[0] = (pos[0] + 1);
        if ((pos[0] >= len)) {
            return {ok: false, error: "unexpected end after minus"};
        }
        ch = str_char_at(expr, pos[0]);
    }
    if ((ch < "0")) {
        return {ok: false, error: "expected number"};
    }
    if ((ch > "9")) {
        return {ok: false, error: "expected number"};
    }
    let result = 0;
    while ((pos[0] < len)) {
        ch = str_char_at(expr, pos[0]);
        if ((ch >= "0")) {
            if ((ch <= "9")) {
                let digit = 0;
                if ((ch === "0")) {
                    digit = 0;
                }
                if ((ch === "1")) {
                    digit = 1;
                }
                if ((ch === "2")) {
                    digit = 2;
                }
                if ((ch === "3")) {
                    digit = 3;
                }
                if ((ch === "4")) {
                    digit = 4;
                }
                if ((ch === "5")) {
                    digit = 5;
                }
                if ((ch === "6")) {
                    digit = 6;
                }
                if ((ch === "7")) {
                    digit = 7;
                }
                if ((ch === "8")) {
                    digit = 8;
                }
                if ((ch === "9")) {
                    digit = 9;
                }
                result = ((result * 10) + digit);
                pos[0] = (pos[0] + 1);
            } else {
                break;
            }
        } else {
            break;
        }
    }
    if (negative) {
        result = (0 - result);
    }
    return {ok: true, value: result};
}

function parse_primary(expr, pos) {
    skip_spaces(expr, pos);
    let len = strlen(expr);
    if ((pos[0] >= len)) {
        return {ok: false, error: "unexpected end of expression"};
    }
    let ch = str_char_at(expr, pos[0]);
    if ((ch === "(")) {
        pos[0] = (pos[0] + 1);
        const _r0 = parse_add(expr, pos);
        if (!_r0.ok) {
            return _r0;
        }
        let val = _r0.value;
        skip_spaces(expr, pos);
        if ((pos[0] >= len)) {
            return {ok: false, error: "missing closing parenthesis"};
        }
        let close = str_char_at(expr, pos[0]);
        if ((close !== ")")) {
            return {ok: false, error: "expected closing parenthesis"};
        }
        pos[0] = (pos[0] + 1);
        return {ok: true, value: val};
    }
    const _r1 = parse_number(expr, pos);
    if (!_r1.ok) {
        return _r1;
    }
    let num = _r1.value;
    return {ok: true, value: num};
}

function parse_mul(expr, pos) {
    const _r2 = parse_primary(expr, pos);
    if (!_r2.ok) {
        return _r2;
    }
    let lhs = _r2.value;
    while ((pos[0] < strlen(expr))) {
        skip_spaces(expr, pos);
        if ((pos[0] >= strlen(expr))) {
            break;
        }
        let op = str_char_at(expr, pos[0]);
        if ((op === "*")) {
            pos[0] = (pos[0] + 1);
            const _r3 = parse_primary(expr, pos);
            if (!_r3.ok) {
                return _r3;
            }
            let rhs = _r3.value;
            lhs = (lhs * rhs);
        } else {
            if ((op === "/")) {
                pos[0] = (pos[0] + 1);
                const _r4 = parse_primary(expr, pos);
                if (!_r4.ok) {
                    return _r4;
                }
                let rhs = _r4.value;
                if ((rhs === 0)) {
                    return {ok: false, error: "division by zero"};
                }
                lhs = Math.trunc(lhs / rhs);
            } else {
                break;
            }
        }
    }
    return {ok: true, value: lhs};
}

function parse_add(expr, pos) {
    const _r5 = parse_mul(expr, pos);
    if (!_r5.ok) {
        return _r5;
    }
    let lhs = _r5.value;
    while ((pos[0] < strlen(expr))) {
        skip_spaces(expr, pos);
        if ((pos[0] >= strlen(expr))) {
            break;
        }
        let op = str_char_at(expr, pos[0]);
        if ((op === "+")) {
            pos[0] = (pos[0] + 1);
            const _r6 = parse_mul(expr, pos);
            if (!_r6.ok) {
                return _r6;
            }
            let rhs = _r6.value;
            lhs = (lhs + rhs);
        } else {
            if ((op === "-")) {
                pos[0] = (pos[0] + 1);
                const _r7 = parse_mul(expr, pos);
                if (!_r7.ok) {
                    return _r7;
                }
                let rhs = _r7.value;
                lhs = (lhs - rhs);
            } else {
                break;
            }
        }
    }
    return {ok: true, value: lhs};
}

function evaluate(expr) {
    let pos = [0];
    const _r8 = parse_add(expr, pos);
    if (!_r8.ok) {
        return _r8;
    }
    let result = _r8.value;
    return {ok: true, value: result};
}

function test_eval(expr) {
    console.log(expr);
    const _r9 = evaluate(expr);
    if (!_r9.ok) {
        console.error("error: " + _r9.error);
        process.exit(1);
    }
    let result = _r9.value;
    console.log(result);
    console.log("");
}

function main() {
    test_eval("3 + 4");
    test_eval("10 - 3");
    test_eval("6 * 7");
    test_eval("20 / 4");
    test_eval("3 + 4 * 2");
    test_eval("(3 + 4) * 2");
    test_eval("(10 + 2) * (8 - 3)");
    test_eval("100 - 20 * 3 + 5");
    test_eval("((2 + 3) * (4 + 1))");
    test_eval("42");
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
