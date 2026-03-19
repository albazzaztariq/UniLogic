
function main() {
    let nums = [3, 1, 4, 1, 5];
    nums.sort((a, b) => a < b ? -1 : a > b ? 1 : 0);
    for (const n of nums) {
        console.log(n);
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
