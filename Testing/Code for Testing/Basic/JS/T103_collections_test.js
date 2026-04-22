
function main() {
    let nums = [10, 20, 30, 40, 50];
    console.log(nums[0]);
    console.log(nums[4]);
    console.log(nums.length);
    let sum = 0;
    for (const n of nums) {
        sum = (sum + n);
    }
    console.log(sum);
    let scores = [];
    scores.push(100);
    scores.push(200);
    scores.push(300);
    console.log(scores.length);
    console.log(scores[0]);
    console.log(scores[2]);
    (scores.splice(1, 0, 150), undefined);
    console.log(scores[1]);
    scores.splice(scores.indexOf(150), 1);
    console.log(scores.length);
    let dropped = scores.splice(0, 1)[0];
    console.log(dropped);
    let items = [5, 10, 15];
    let item_sum = 0;
    for (const it of items) {
        item_sum = (item_sum + it);
    }
    console.log(item_sum);
    let sliced = nums.slice(1, 4);
    console.log(sliced[0]);
    console.log(sliced[1]);
    console.log(sliced[2]);
    if (nums.includes(30)) {
        console.log("found");
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
