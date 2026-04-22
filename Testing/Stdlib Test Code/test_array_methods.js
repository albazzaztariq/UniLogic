
function array_sum(arr, n) {
    let total = 0;
    let i = 0;
    while ((i < n)) {
        total = (total + arr[i]);
        i = (i + 1);
    }
    return total;
}

function array_contains(arr, n, val) {
    let i = 0;
    while ((i < n)) {
        if ((arr[i] === val)) {
            return true;
        }
        i = (i + 1);
    }
    return false;
}

function main() {
    let nums = [3, 1, 4, 1, 5];
    let total = nums.reduce((a, b) => a + b, 0);
    console.log(total);
    let found = nums.includes(4);
    console.log(found);
    let nope = nums.includes(9);
    console.log(nope);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
