const Priority = Object.freeze({low: 1, medium: 2, high: 3});
const Color = Object.freeze({red: 10, green: 20, blue: 30});

const VERSION = 1;
const APP = "TestApp";
function main() {
    console.log(VERSION);
    console.log(APP);
    let p = Priority.high;
    console.log(p);
    let c = Color.green;
    console.log(c);
    if ((p === 3)) {
        console.log("high priority confirmed");
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
