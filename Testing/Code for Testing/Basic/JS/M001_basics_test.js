
function main() {
    let x = 10;
    let y = 20;
    let sum = (x + y);
    console.log(sum);
    let pi = 3.14;
    console.log(pi);
    let name = "UniLogic";
    console.log(name);
    let flag = true;
    console.log(flag);
    let a = 100;
    a += 50;
    a -= 25;
    a *= 2;
    console.log(a);
    let mod = (17 % 5);
    console.log(mod);
    let greeting = ("Hello " + "World");
    console.log(greeting);
    const MAX = 999;
    console.log(MAX);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
