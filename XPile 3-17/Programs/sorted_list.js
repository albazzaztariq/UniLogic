
function bisect_arr(arr, lo, hi, val) {
    while ((lo < hi)) {
        let mid = Math.trunc((lo + hi) / 2);
        if ((arr[mid] < val)) {
            lo = (mid + 1);
        } else {
            hi = mid;
        }
    }
    return lo;
}

function bisect_arr_r(arr, lo, hi, val) {
    while ((lo < hi)) {
        let mid = Math.trunc((lo + hi) / 2);
        if ((arr[mid] <= val)) {
            lo = (mid + 1);
        } else {
            hi = mid;
        }
    }
    return lo;
}

function shift_up(data, pos, end_idx) {
    let i = end_idx;
    while ((i > pos)) {
        data[i] = data[(i - 1)];
        i = (i - 1);
    }
}

function shift_down(data, pos, end_idx) {
    let i = pos;
    while ((i < (end_idx - 1))) {
        data[i] = data[(i + 1)];
        i = (i + 1);
    }
}

function shift_data_up(data, from_pos, total) {
    let i = total;
    while ((i > from_pos)) {
        data[i] = data[(i - 1)];
        i = (i - 1);
    }
}

function sl_insort(data, start, len, val) {
    let pos = bisect_arr(data, start, (start + len), val);
    shift_up(data, pos, (start + len));
    data[pos] = val;
    return pos;
}

function sl_expand(data, sub_start, sub_len, sub_max, state, pos) {
    let load = state[2];
    let num_subs = state[0];
    if ((sub_len[pos] > (load * 2))) {
        let old_start = sub_start[pos];
        let split_at = load;
        let i = num_subs;
        while ((i > (pos + 1))) {
            sub_start[i] = sub_start[(i - 1)];
            sub_len[i] = sub_len[(i - 1)];
            sub_max[i] = sub_max[(i - 1)];
            i = (i - 1);
        }
        sub_start[(pos + 1)] = (old_start + split_at);
        sub_len[(pos + 1)] = (sub_len[pos] - split_at);
        sub_max[(pos + 1)] = data[((old_start + sub_len[pos]) - 1)];
        sub_len[pos] = split_at;
        sub_max[pos] = data[((old_start + split_at) - 1)];
        state[0] = (num_subs + 1);
    }
}

function sl_add(data, sub_start, sub_len, sub_max, state, val) {
    let num_subs = state[0];
    let total = state[1];
    if ((num_subs === 0)) {
        data[0] = val;
        sub_start[0] = 0;
        sub_len[0] = 1;
        sub_max[0] = val;
        state[0] = 1;
        state[1] = 1;
        return;
    }
    let pos = 0;
    for (const k of __ul_range(num_subs)) {
        if ((sub_max[k] < val)) {
            pos = (k + 1);
        } else {
            break;
        }
    }
    if ((pos === num_subs)) {
        pos = (num_subs - 1);
        let ins_at = (sub_start[pos] + sub_len[pos]);
        let i = total;
        while ((i > ins_at)) {
            data[i] = data[(i - 1)];
            i = (i - 1);
        }
        data[ins_at] = val;
        sub_len[pos] = (sub_len[pos] + 1);
        sub_max[pos] = val;
        for (const s of __ul_range((pos + 1), num_subs)) {
            sub_start[s] = (sub_start[s] + 1);
        }
    } else {
        let s_start = sub_start[pos];
        let s_len = sub_len[pos];
        let ins_pos = bisect_arr(data, s_start, (s_start + s_len), val);
        let i = total;
        while ((i > ins_pos)) {
            data[i] = data[(i - 1)];
            i = (i - 1);
        }
        data[ins_pos] = val;
        sub_len[pos] = (sub_len[pos] + 1);
        if ((val > sub_max[pos])) {
            sub_max[pos] = val;
        }
        for (const s of __ul_range((pos + 1), num_subs)) {
            sub_start[s] = (sub_start[s] + 1);
        }
    }
    state[1] = (total + 1);
    sl_expand(data, sub_start, sub_len, sub_max, state, pos);
}

function sl_remove(data, sub_start, sub_len, sub_max, state, val) {
    let num_subs = state[0];
    if ((num_subs === 0)) {
        return {ok: false, error: "value not in list"};
    }
    let pos = 0;
    for (const k of __ul_range(num_subs)) {
        if ((sub_max[k] < val)) {
            pos = (k + 1);
        }
    }
    if ((pos === num_subs)) {
        return {ok: false, error: "value not in list"};
    }
    let s_start = sub_start[pos];
    let s_len = sub_len[pos];
    let idx = bisect_arr(data, s_start, (s_start + s_len), val);
    if ((idx >= (s_start + s_len))) {
        return {ok: false, error: "value not in list"};
    }
    if ((data[idx] !== val)) {
        return {ok: false, error: "value not in list"};
    }
    let total = state[1];
    let i = idx;
    while ((i < (total - 1))) {
        data[i] = data[(i + 1)];
        i = (i + 1);
    }
    sub_len[pos] = (sub_len[pos] - 1);
    state[1] = (total - 1);
    for (const s of __ul_range((pos + 1), num_subs)) {
        sub_start[s] = (sub_start[s] - 1);
    }
    if ((sub_len[pos] === 0)) {
        let j = pos;
        while ((j < (num_subs - 1))) {
            sub_start[j] = sub_start[(j + 1)];
            sub_len[j] = sub_len[(j + 1)];
            sub_max[j] = sub_max[(j + 1)];
            j = (j + 1);
        }
        state[0] = (num_subs - 1);
    } else {
        sub_max[pos] = data[((sub_start[pos] + sub_len[pos]) - 1)];
    }
    return {ok: true, value: 1};
}

function sl_contains(data, sub_start, sub_len, sub_max, state, val) {
    let num_subs = state[0];
    if ((num_subs === 0)) {
        return false;
    }
    let pos = 0;
    for (const k of __ul_range(num_subs)) {
        if ((sub_max[k] < val)) {
            pos = (k + 1);
        }
    }
    if ((pos === num_subs)) {
        return false;
    }
    let s_start = sub_start[pos];
    let s_len = sub_len[pos];
    let idx = bisect_arr(data, s_start, (s_start + s_len), val);
    if ((idx >= (s_start + s_len))) {
        return false;
    }
    return (data[idx] === val);
}

function sl_get(data, sub_start, sub_len, state, idx) {
    let remaining = idx;
    let num_subs = state[0];
    for (const s of __ul_range(num_subs)) {
        if ((remaining < sub_len[s])) {
            return data[(sub_start[s] + remaining)];
        }
        remaining = (remaining - sub_len[s]);
    }
    return 0;
}

function sl_len(state) {
    return state[1];
}

function sl_print(data, state) {
    let total = state[1];
    let result = "[";
    for (const i of __ul_range(total)) {
        if ((i > 0)) {
            result = (result + ", ");
        }
        result = (result + "{data[i]}");
    }
    result = (result + "]");
    console.log(result);
}

function main() {
    let data = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    let sub_start = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    let sub_len = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    let sub_max = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    let state = [0, 0, 8];
    console.log("Adding: 50, 20, 80, 10, 40, 90, 30, 70, 60, 15");
    sl_add(data, sub_start, sub_len, sub_max, state, 50);
    sl_add(data, sub_start, sub_len, sub_max, state, 20);
    sl_add(data, sub_start, sub_len, sub_max, state, 80);
    sl_add(data, sub_start, sub_len, sub_max, state, 10);
    sl_add(data, sub_start, sub_len, sub_max, state, 40);
    sl_add(data, sub_start, sub_len, sub_max, state, 90);
    sl_add(data, sub_start, sub_len, sub_max, state, 30);
    sl_add(data, sub_start, sub_len, sub_max, state, 70);
    sl_add(data, sub_start, sub_len, sub_max, state, 60);
    sl_add(data, sub_start, sub_len, sub_max, state, 15);
    console.log("After 10 adds:");
    sl_print(data, state);
    console.log("Length:");
    console.log(sl_len(state));
    console.log("Contains 40:");
    console.log(sl_contains(data, sub_start, sub_len, sub_max, state, 40));
    console.log("Contains 99:");
    console.log(sl_contains(data, sub_start, sub_len, sub_max, state, 99));
    console.log("Contains 10:");
    console.log(sl_contains(data, sub_start, sub_len, sub_max, state, 10));
    console.log("Contains 90:");
    console.log(sl_contains(data, sub_start, sub_len, sub_max, state, 90));
    console.log("Element at index 0:");
    console.log(sl_get(data, sub_start, sub_len, state, 0));
    console.log("Element at index 5:");
    console.log(sl_get(data, sub_start, sub_len, state, 5));
    console.log("Element at index 9:");
    console.log(sl_get(data, sub_start, sub_len, state, 9));
    console.log("Removing 20, 80, 15");
    const _r0 = sl_remove(data, sub_start, sub_len, sub_max, state, 20);
    if (!_r0.ok) {
        console.error("error: " + _r0.error);
        process.exit(1);
    }
    _r0.value;
    const _r1 = sl_remove(data, sub_start, sub_len, sub_max, state, 80);
    if (!_r1.ok) {
        console.error("error: " + _r1.error);
        process.exit(1);
    }
    _r1.value;
    const _r2 = sl_remove(data, sub_start, sub_len, sub_max, state, 15);
    if (!_r2.ok) {
        console.error("error: " + _r2.error);
        process.exit(1);
    }
    _r2.value;
    console.log("After removes:");
    sl_print(data, state);
    console.log("Length:");
    console.log(sl_len(state));
    console.log("Adding: 25, 25, 55, 5, 95, 42");
    sl_add(data, sub_start, sub_len, sub_max, state, 25);
    sl_add(data, sub_start, sub_len, sub_max, state, 25);
    sl_add(data, sub_start, sub_len, sub_max, state, 55);
    sl_add(data, sub_start, sub_len, sub_max, state, 5);
    sl_add(data, sub_start, sub_len, sub_max, state, 95);
    sl_add(data, sub_start, sub_len, sub_max, state, 42);
    console.log("Final state:");
    sl_print(data, state);
    console.log("Length:");
    console.log(sl_len(state));
    console.log("Elements by index:");
    let total = sl_len(state);
    for (const i of __ul_range(total)) {
        console.log(sl_get(data, sub_start, sub_len, state, i));
    }
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
