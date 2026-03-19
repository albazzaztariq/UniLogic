
function main() {
    let greeting = "Hello World";
    let hello = greeting.slice(0, 5);
    console.log(hello);
    let world = greeting.slice(6, 11);
    console.log(world);
    let nums = [10, 20, 30, 40, 50];
    let middle = nums.slice(1, 4);
    console.log(middle[0]);
    console.log(middle[1]);
    console.log(middle[2]);
    let h = greeting.slice(0, 1);
    console.log(h);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
