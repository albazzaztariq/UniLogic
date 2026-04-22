
function main() {
    let count = 0;
    do {
        console.log(count);
        count += 1;
    } while ((count < 5));
    let x = 100;
    do {
        console.log(x);
        x += 1;
    } while ((x < 0));
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
