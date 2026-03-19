// @dr memory = manual

function str_char_at(s, i) { return i >= 0 && i < s.length ? s[i] : ''; }
function strlen(s) { return s.length; }


function char_val(ch) {
    if ((ch === "a")) {
        return 97;
    }
    if ((ch === "b")) {
        return 98;
    }
    if ((ch === "c")) {
        return 99;
    }
    if ((ch === "d")) {
        return 100;
    }
    if ((ch === "e")) {
        return 101;
    }
    if ((ch === "f")) {
        return 102;
    }
    if ((ch === "g")) {
        return 103;
    }
    if ((ch === "h")) {
        return 104;
    }
    if ((ch === "i")) {
        return 105;
    }
    if ((ch === "j")) {
        return 106;
    }
    if ((ch === "k")) {
        return 107;
    }
    if ((ch === "l")) {
        return 108;
    }
    if ((ch === "m")) {
        return 109;
    }
    if ((ch === "n")) {
        return 110;
    }
    if ((ch === "o")) {
        return 111;
    }
    if ((ch === "p")) {
        return 112;
    }
    if ((ch === "q")) {
        return 113;
    }
    if ((ch === "r")) {
        return 114;
    }
    if ((ch === "s")) {
        return 115;
    }
    if ((ch === "t")) {
        return 116;
    }
    if ((ch === "u")) {
        return 117;
    }
    if ((ch === "v")) {
        return 118;
    }
    if ((ch === "w")) {
        return 119;
    }
    if ((ch === "x")) {
        return 120;
    }
    if ((ch === "y")) {
        return 121;
    }
    if ((ch === "z")) {
        return 122;
    }
    if ((ch === "A")) {
        return 65;
    }
    if ((ch === "B")) {
        return 66;
    }
    if ((ch === "C")) {
        return 67;
    }
    if ((ch === "D")) {
        return 68;
    }
    if ((ch === "E")) {
        return 69;
    }
    if ((ch === "F")) {
        return 70;
    }
    if ((ch === "G")) {
        return 71;
    }
    if ((ch === "H")) {
        return 72;
    }
    if ((ch === "I")) {
        return 73;
    }
    if ((ch === "J")) {
        return 74;
    }
    if ((ch === "K")) {
        return 75;
    }
    if ((ch === "L")) {
        return 76;
    }
    if ((ch === "M")) {
        return 77;
    }
    if ((ch === "N")) {
        return 78;
    }
    if ((ch === "O")) {
        return 79;
    }
    if ((ch === "P")) {
        return 80;
    }
    if ((ch === "Q")) {
        return 81;
    }
    if ((ch === "R")) {
        return 82;
    }
    if ((ch === "S")) {
        return 83;
    }
    if ((ch === "T")) {
        return 84;
    }
    if ((ch === "U")) {
        return 85;
    }
    if ((ch === "V")) {
        return 86;
    }
    if ((ch === "W")) {
        return 87;
    }
    if ((ch === "X")) {
        return 88;
    }
    if ((ch === "Y")) {
        return 89;
    }
    if ((ch === "Z")) {
        return 90;
    }
    if ((ch === "0")) {
        return 48;
    }
    if ((ch === "1")) {
        return 49;
    }
    if ((ch === "2")) {
        return 50;
    }
    if ((ch === "3")) {
        return 51;
    }
    if ((ch === "4")) {
        return 52;
    }
    if ((ch === "5")) {
        return 53;
    }
    if ((ch === "6")) {
        return 54;
    }
    if ((ch === "7")) {
        return 55;
    }
    if ((ch === "8")) {
        return 56;
    }
    if ((ch === "9")) {
        return 57;
    }
    if ((ch === " ")) {
        return 32;
    }
    if ((ch === "_")) {
        return 95;
    }
    if ((ch === "-")) {
        return 45;
    }
    if ((ch === ".")) {
        return 46;
    }
    return 0;
}

function hash_key(key) {
    let hash = 5381;
    let len = strlen(key);
    for (const i of __ul_range(len)) {
        let ch = str_char_at(key, i);
        hash = ((hash * 31) + char_val(ch));
    }
    if ((hash < 0)) {
        hash = (0 - hash);
    }
    return hash;
}

function hm_set(keys, vals, hm_state, key, val) {
    let cap = hm_state[0];
    if ((hm_state[1] >= Math.trunc(cap / 2))) {
        let new_cap = (cap * 2);
        let old_keys = ["", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""];
        let old_vals = ["", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""];
        for (const i of __ul_range(cap)) {
            old_keys[i] = keys[i];
            old_vals[i] = vals[i];
            keys[i] = "";
            vals[i] = "";
        }
        hm_state[0] = new_cap;
        hm_state[1] = 0;
        for (const i of __ul_range(cap)) {
            if ((old_keys[i] !== "")) {
                if ((old_keys[i] !== "__deleted__")) {
                    hm_set(keys, vals, hm_state, old_keys[i], old_vals[i]);
                }
            }
        }
        cap = new_cap;
    }
    let hash = hash_key(key);
    let index = (hash % cap);
    while ((keys[index] !== "")) {
        if ((keys[index] === key)) {
            vals[index] = val;
            return;
        }
        if ((keys[index] === "__deleted__")) {
            break;
        }
        index = (index + 1);
        if ((index >= cap)) {
            index = 0;
        }
    }
    keys[index] = key;
    vals[index] = val;
    hm_state[1] = (hm_state[1] + 1);
}

function hm_get(keys, vals, hm_state, key) {
    let cap = hm_state[0];
    let hash = hash_key(key);
    let index = (hash % cap);
    while ((keys[index] !== "")) {
        if ((keys[index] === key)) {
            return {ok: true, value: vals[index]};
        }
        index = (index + 1);
        if ((index >= cap)) {
            index = 0;
        }
    }
    return {ok: false, error: "key not found"};
}

function hm_has(keys, hm_state, key) {
    let cap = hm_state[0];
    let hash = hash_key(key);
    let index = (hash % cap);
    while ((keys[index] !== "")) {
        if ((keys[index] === key)) {
            return true;
        }
        index = (index + 1);
        if ((index >= cap)) {
            index = 0;
        }
    }
    return false;
}

function hm_delete(keys, vals, hm_state, key) {
    let cap = hm_state[0];
    let hash = hash_key(key);
    let index = (hash % cap);
    while ((keys[index] !== "")) {
        if ((keys[index] === key)) {
            keys[index] = "__deleted__";
            vals[index] = "";
            hm_state[1] = (hm_state[1] - 1);
            return;
        }
        index = (index + 1);
        if ((index >= cap)) {
            index = 0;
        }
    }
}

function hm_len(hm_state) {
    return hm_state[1];
}

function main() {
    let keys = ["", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""];
    let vals = ["", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""];
    let hm_state = [16, 0];
    console.log("Setting 10 entries...");
    hm_set(keys, vals, hm_state, "apple", "red");
    hm_set(keys, vals, hm_state, "banana", "yellow");
    hm_set(keys, vals, hm_state, "cherry", "red");
    hm_set(keys, vals, hm_state, "date", "brown");
    hm_set(keys, vals, hm_state, "elderberry", "purple");
    hm_set(keys, vals, hm_state, "fig", "green");
    hm_set(keys, vals, hm_state, "grape", "purple");
    hm_set(keys, vals, hm_state, "honeydew", "green");
    hm_set(keys, vals, hm_state, "kiwi", "brown");
    hm_set(keys, vals, hm_state, "lemon", "yellow");
    console.log("Length:");
    console.log(hm_len(hm_state));
    console.log("");
    console.log("Get tests:");
    const _r0 = hm_get(keys, vals, hm_state, "apple");
    if (!_r0.ok) {
        console.error("error: " + _r0.error);
        process.exit(1);
    }
    let v1 = _r0.value;
    console.log("apple -> {v1}");
    const _r1 = hm_get(keys, vals, hm_state, "banana");
    if (!_r1.ok) {
        console.error("error: " + _r1.error);
        process.exit(1);
    }
    let v2 = _r1.value;
    console.log("banana -> {v2}");
    const _r2 = hm_get(keys, vals, hm_state, "grape");
    if (!_r2.ok) {
        console.error("error: " + _r2.error);
        process.exit(1);
    }
    let v3 = _r2.value;
    console.log("grape -> {v3}");
    const _r3 = hm_get(keys, vals, hm_state, "kiwi");
    if (!_r3.ok) {
        console.error("error: " + _r3.error);
        process.exit(1);
    }
    let v4 = _r3.value;
    console.log("kiwi -> {v4}");
    const _r4 = hm_get(keys, vals, hm_state, "lemon");
    if (!_r4.ok) {
        console.error("error: " + _r4.error);
        process.exit(1);
    }
    let v5 = _r4.value;
    console.log("lemon -> {v5}");
    console.log("");
    console.log("Has tests:");
    console.log("has apple:");
    console.log(hm_has(keys, hm_state, "apple"));
    console.log("has mango:");
    console.log(hm_has(keys, hm_state, "mango"));
    console.log("has cherry:");
    console.log(hm_has(keys, hm_state, "cherry"));
    console.log("has zzz:");
    console.log(hm_has(keys, hm_state, "zzz"));
    console.log("has fig:");
    console.log(hm_has(keys, hm_state, "fig"));
    console.log("");
    console.log("Updating 5 entries...");
    hm_set(keys, vals, hm_state, "apple", "green");
    hm_set(keys, vals, hm_state, "banana", "brown");
    hm_set(keys, vals, hm_state, "cherry", "dark red");
    hm_set(keys, vals, hm_state, "grape", "green");
    hm_set(keys, vals, hm_state, "lemon", "bright yellow");
    console.log("Length after updates (should be same):");
    console.log(hm_len(hm_state));
    const _r5 = hm_get(keys, vals, hm_state, "apple");
    if (!_r5.ok) {
        console.error("error: " + _r5.error);
        process.exit(1);
    }
    let v6 = _r5.value;
    console.log("apple -> {v6}");
    const _r6 = hm_get(keys, vals, hm_state, "cherry");
    if (!_r6.ok) {
        console.error("error: " + _r6.error);
        process.exit(1);
    }
    let v7 = _r6.value;
    console.log("cherry -> {v7}");
    console.log("");
    console.log("Deleting fig, date, elderberry");
    hm_delete(keys, vals, hm_state, "fig");
    hm_delete(keys, vals, hm_state, "date");
    hm_delete(keys, vals, hm_state, "elderberry");
    console.log("Length after deletes:");
    console.log(hm_len(hm_state));
    console.log("has fig:");
    console.log(hm_has(keys, hm_state, "fig"));
    console.log("has date:");
    console.log(hm_has(keys, hm_state, "date"));
    console.log("");
    console.log("Adding 10 more entries...");
    hm_set(keys, vals, hm_state, "mango", "orange");
    hm_set(keys, vals, hm_state, "nectarine", "orange");
    hm_set(keys, vals, hm_state, "orange", "orange");
    hm_set(keys, vals, hm_state, "papaya", "orange");
    hm_set(keys, vals, hm_state, "quince", "yellow");
    hm_set(keys, vals, hm_state, "raspberry", "red");
    hm_set(keys, vals, hm_state, "strawberry", "red");
    hm_set(keys, vals, hm_state, "tangerine", "orange");
    hm_set(keys, vals, hm_state, "watermelon", "green");
    hm_set(keys, vals, hm_state, "plum", "purple");
    console.log("Length:");
    console.log(hm_len(hm_state));
    console.log("");
    console.log("Verify after expansion:");
    const _r7 = hm_get(keys, vals, hm_state, "apple");
    if (!_r7.ok) {
        console.error("error: " + _r7.error);
        process.exit(1);
    }
    let va = _r7.value;
    console.log("apple -> {va}");
    const _r8 = hm_get(keys, vals, hm_state, "banana");
    if (!_r8.ok) {
        console.error("error: " + _r8.error);
        process.exit(1);
    }
    let vb = _r8.value;
    console.log("banana -> {vb}");
    const _r9 = hm_get(keys, vals, hm_state, "mango");
    if (!_r9.ok) {
        console.error("error: " + _r9.error);
        process.exit(1);
    }
    let vm = _r9.value;
    console.log("mango -> {vm}");
    const _r10 = hm_get(keys, vals, hm_state, "orange");
    if (!_r10.ok) {
        console.error("error: " + _r10.error);
        process.exit(1);
    }
    let vo = _r10.value;
    console.log("orange -> {vo}");
    const _r11 = hm_get(keys, vals, hm_state, "raspberry");
    if (!_r11.ok) {
        console.error("error: " + _r11.error);
        process.exit(1);
    }
    let vr = _r11.value;
    console.log("raspberry -> {vr}");
    const _r12 = hm_get(keys, vals, hm_state, "watermelon");
    if (!_r12.ok) {
        console.error("error: " + _r12.error);
        process.exit(1);
    }
    let vw = _r12.value;
    console.log("watermelon -> {vw}");
    const _r13 = hm_get(keys, vals, hm_state, "plum");
    if (!_r13.ok) {
        console.error("error: " + _r13.error);
        process.exit(1);
    }
    let vp = _r13.value;
    console.log("plum -> {vp}");
    console.log("has strawberry:");
    console.log(hm_has(keys, hm_state, "strawberry"));
    console.log("has fig (deleted):");
    console.log(hm_has(keys, hm_state, "fig"));
    console.log("Final length:");
    console.log(hm_len(hm_state));
    return 0;
}


function __ul_range(...args) {
    let start, end, step;
    if (args.length === 1) { start = 0; end = args[0]; step = 1; }
    else if (args.length === 2) { start = args[0]; end = args[1]; step = 1; }
    else { start = args[0]; end = args[1]; step = args[2]; }
    const result = [];
    for (let i = start; step > 0 ? i < end : i > end; i += step) result.push(i);
    return result;
}
function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
