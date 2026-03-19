const ceil = Math.ceil;
const floor = Math.floor;
const pow = Math.pow;
const sqrt = Math.sqrt;


function math_sqrt(x) {
    return sqrt(x);
}

function math_floor(x) {
    return floor(x);
}

function sort_doubles(arr, n) {
    for (const i of __ul_range(1, n)) {
        let key = arr[i];
        let j = (i - 1);
        while ((j >= 0)) {
            if ((arr[j] > key)) {
                arr[(j + 1)] = arr[j];
                j = (j - 1);
            } else {
                break;
            }
        }
        arr[(j + 1)] = key;
    }
}

function stat_mean(arr, n) {
    let sum = 0.0;
    for (const i of __ul_range(n)) {
        sum = (sum + arr[i]);
    }
    return (sum / Number(n));
}

function quantile_sorted(sorted, n, p) {
    if ((p === 0.0)) {
        return sorted[0];
    }
    if ((p === 1.0)) {
        return sorted[(n - 1)];
    }
    let idx = (Number((n - 1)) * p);
    let fl = math_floor(idx);
    let lo = Math.trunc(fl);
    let fraction = (idx - fl);
    if ((fraction < 0.0001)) {
        return sorted[lo];
    }
    return (sorted[lo] + (fraction * (sorted[(lo + 1)] - sorted[lo])));
}

function stat_quantile(arr, n, p) {
    let sorted = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
    for (const i of __ul_range(n)) {
        sorted[i] = arr[i];
    }
    sort_doubles(sorted, n);
    return quantile_sorted(sorted, n, p);
}

function stat_median(arr, n) {
    return stat_quantile(arr, n, 0.5);
}

function stat_mode(arr, n) {
    let sorted = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
    for (const i of __ul_range(n)) {
        sorted[i] = arr[i];
    }
    sort_doubles(sorted, n);
    let max_val = sorted[0];
    let max_count = 0;
    let count = 0;
    let last_val = sorted[0];
    for (const i of __ul_range(n)) {
        if ((sorted[i] === last_val)) {
            count = (count + 1);
        } else {
            count = 1;
            last_val = sorted[i];
        }
        if ((count > max_count)) {
            max_count = count;
            max_val = sorted[i];
        }
    }
    return max_val;
}

function stat_variance(arr, n) {
    let m = stat_mean(arr, n);
    let sum_sq = 0.0;
    for (const i of __ul_range(n)) {
        let dev = (arr[i] - m);
        sum_sq = (sum_sq + (dev * dev));
    }
    return (sum_sq / Number(n));
}

function stat_stddev(arr, n) {
    if ((n === 1)) {
        return 0.0;
    }
    let v = stat_variance(arr, n);
    return math_sqrt(v);
}

function main() {
    console.log("=== Dataset 1: [2, 4, 4, 4, 5, 5, 7, 9] ===");
    let d1 = [2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
    let n1 = 8;
    console.log("mean:");
    console.log(stat_mean(d1, n1));
    console.log("median:");
    console.log(stat_median(d1, n1));
    console.log("mode:");
    console.log(stat_mode(d1, n1));
    console.log("variance:");
    console.log(stat_variance(d1, n1));
    console.log("stddev:");
    console.log(stat_stddev(d1, n1));
    console.log("");
    console.log("=== Dataset 2: [1, 2, 3, 4, 5] ===");
    let d2 = [1.0, 2.0, 3.0, 4.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
    let n2 = 5;
    console.log("mean:");
    console.log(stat_mean(d2, n2));
    console.log("median:");
    console.log(stat_median(d2, n2));
    console.log("mode:");
    console.log(stat_mode(d2, n2));
    console.log("variance:");
    console.log(stat_variance(d2, n2));
    console.log("stddev:");
    console.log(stat_stddev(d2, n2));
    console.log("");
    console.log("=== Quantile tests ===");
    let d3 = [3.0, 6.0, 7.0, 8.0, 8.0, 9.0, 10.0, 13.0, 15.0, 16.0, 20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
    let n3 = 11;
    console.log("Q(0.25):");
    console.log(stat_quantile(d3, n3, 0.25));
    console.log("Q(0.5):");
    console.log(stat_quantile(d3, n3, 0.5));
    console.log("Q(0.75):");
    console.log(stat_quantile(d3, n3, 0.75));
    console.log("Q(0.0):");
    console.log(stat_quantile(d3, n3, 0.0));
    console.log("Q(1.0):");
    console.log(stat_quantile(d3, n3, 1.0));
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
