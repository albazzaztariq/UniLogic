// @packed - no padding (packed struct)
// bit field: nibble_hi = 4 bits
// bit field: nibble_lo = 4 bits
function createPackedData() {
    return { nibble_hi: 0, nibble_lo: 0, byte_val: 0 };
}


function main() {
    let d = createPackedData();
    d.nibble_hi = 15;
    d.nibble_lo = 9;
    d.byte_val = 200;
    console.log(d.nibble_hi);
    console.log(d.nibble_lo);
    console.log(d.byte_val);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
