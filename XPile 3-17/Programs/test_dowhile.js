
function main() {
    let count = 0;
    do {
        count = (count + 1);
    } while ((count < 5));
    console.log(count);
    let x = 10;
    do {
        x = (x + 1);
    } while ((x < 5));
    console.log(x);
    let n = 0;
    do {
        n = (n + 1);
        if ((n === 3)) {
            break;
        }
    } while ((n < 10));
    console.log(n);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
