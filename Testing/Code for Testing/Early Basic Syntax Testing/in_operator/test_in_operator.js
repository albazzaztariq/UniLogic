
function main() {
    let nums = [10, 20, 30, 40, 50];
    if (nums.includes(30)) {
        console.log("found 30");
    }
    if (nums.includes(99)) {
        console.log("should not print");
    }
    let fruits = ["apple", "banana", "mango"];
    if (fruits.includes("banana")) {
        console.log("found banana");
    }
    if (fruits.includes("grape")) {
        console.log("should not print");
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
