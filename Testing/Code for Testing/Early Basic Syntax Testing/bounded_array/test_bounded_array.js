
function main() {
    let nums = [10, 20, 30, 40, 50];
    console.log(nums[0]);
    console.log(nums[4]);
    let buf = new Array(3).fill(0);
    buf[0] = 100;
    buf[1] = 200;
    buf[2] = 300;
    console.log(buf[0]);
    console.log(buf[2]);
    let vals = [1, 2, 3];
    console.log(vals[0]);
    console.log(vals[2]);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
