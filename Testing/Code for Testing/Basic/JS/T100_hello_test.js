
function main() {
    console.log("Hello, UniLogic!");
    let x = 42;
    console.log(x);
    let pi = 3.14;
    console.log(pi);
    let name = "UniLogic";
    console.log(name);
    let flag = true;
    console.log(flag);
    let sum = (10 + 20);
    console.log(sum);
    let greeting = ("Hello " + "World");
    console.log(greeting);
    let a = 5;
    let b = 10;
    let c = (a + b);
    console.log(c);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
