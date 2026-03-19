// @dr memory = refcount


function allocate_many() {
    let total = 0;
    let i = 0;
    while ((i < 1000)) {
        let s = ("item " + String(i));
        total = (total + str_len(s));
        i = (i + 1);
    }
    return total;
}

function main() {
    let result = allocate_many();
    console.log(result);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
