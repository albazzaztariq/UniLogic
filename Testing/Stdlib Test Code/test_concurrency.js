// @dr concurrency = threaded


function worker(id) {
    console.log(id);
}

function main() {
    __ul_threads.push(() => worker(1));
    (__ul_threads.length - 1);
    __ul_threads.push(() => worker(2));
    (__ul_threads.length - 1);
    __ul_threads.push(() => worker(3));
    (__ul_threads.length - 1);
    for (const __t of __ul_threads) __t();
    __ul_threads.length = 0;
    undefined;
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }
const __ul_threads = [];

main();
