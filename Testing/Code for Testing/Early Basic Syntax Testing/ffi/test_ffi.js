const sqrt = Math.sqrt;
function puts(s) { console.log(s); return 0; }


function main() {
    let val = 144.0;
    let result = sqrt(val);
    console.log(result);
    puts("Hello from FFI!");
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
