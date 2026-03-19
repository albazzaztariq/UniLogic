
function main() {
    let scores = [];
    scores.push(10);
    scores.push(20);
    scores.push(30);
    console.log(scores.length);
    console.log(scores[0]);
    console.log(scores[1]);
    console.log(scores[2]);
    (scores.splice(1, 0, 15), undefined);
    console.log(scores[1]);
    console.log(scores.length);
    scores.splice(scores.indexOf(15), 1);
    console.log(scores.length);
    let dropped = scores.splice(0, 1)[0];
    console.log(dropped);
    console.log(scores.length);
    let nums = [5, 3, 1, 4, 2];
    console.log(nums.length);
    nums.sort((a, b) => a < b ? -1 : a > b ? 1 : 0);
    console.log(nums[0]);
    console.log(nums[4]);
    nums.reverse();
    console.log(nums[0]);
    console.log(nums[4]);
    if (nums.includes(3)) {
        console.log("found 3");
    }
    (nums.length = 0);
    console.log(nums.length);
    let items = [100, 200, 300];
    let total = 0;
    for (const x of items) {
        total = (total + x);
    }
    console.log(total);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
