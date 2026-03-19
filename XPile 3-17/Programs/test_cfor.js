
function main() {
    let sum1 = 0;
    for (let i = 0; (i < 5); i++) {
        sum1 = (sum1 + i);
    }
    console.log(sum1);
    let sum2 = 0;
    for (let j = 1; (j <= 5); j++) {
        sum2 = (sum2 + j);
    }
    console.log(sum2);
    let last = 0;
    for (let k = 0; (k < 100); k++) {
        if ((k === 7)) {
            last = k;
            break;
        }
    }
    console.log(last);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
