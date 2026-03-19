
function main() {
    let nums = [];
    nums.push(10);
    nums.push(20);
    nums.push(30);
    console.log(nums.length);
    console.log(nums[0]);
    console.log(nums[2]);
    let popped = nums.pop();
    console.log(popped);
    console.log(nums.length);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
