
function validate(x) {
    if ((x < 0)) {
        return (-1);
    }
    return x;
}

function main() {
    let val = validate(10);
    if ((val < 0)) {
        console.log(0);
        return 1;
    }
    console.log(val);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
