
function main() {
    let scores = [95, 87, 72, 100, 63];
    console.log(scores[0]);
    console.log(scores[4]);
    let names = ["Alice", "Bob", "Carol"];
    names.push("Dave");
    console.log(names[0]);
    console.log(names[3]);
    let total = 0;
    for (const s of scores) {
        total += s;
    }
    console.log(total);
    for (const name of names) {
        console.log(name);
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
