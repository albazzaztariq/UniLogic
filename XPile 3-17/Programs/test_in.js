
function main() {
    let msg = "hello world";
    if (msg.includes("world")) {
        console.log(1);
    }
    if (msg.includes("xyz")) {
        console.log(0);
    } else {
        console.log(1);
    }
    let nums = [10, 20, 30, 40, 50];
    if (nums.includes(30)) {
        console.log(1);
    }
    if (nums.includes(99)) {
        console.log(0);
    } else {
        console.log(1);
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
