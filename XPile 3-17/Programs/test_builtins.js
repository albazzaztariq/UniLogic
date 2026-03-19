
function double_it(x) {
    return (x * 2);
}

function is_even(x) {
    return ((x % 2) === 0);
}

function main() {
    let nums = [1, 2, 3, 4, 5];
    let doubled = nums.map(double_it);
    for (const d of doubled) {
        console.log(d);
    }
    let evens = nums.filter(is_even);
    for (const e of evens) {
        console.log(e);
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
