
function get_resource() {
    return 42;
}

function main() {
    let r = get_resource();
    try {
        console.log(r);
    } finally {
        if (r && typeof r.close === 'function') r.close();
    }
    let val = get_resource();
    try {
        let doubled = (val * 2);
        console.log(doubled);
    } finally {
        if (val && typeof val.close === 'function') val.close();
    }
    console.log(1);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
