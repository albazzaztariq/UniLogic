// @dr concurrency = threaded


let shared_counter = 0;
function worker() {
    let i = 0;
    while ((i < 1000)) {
        lock(counter_lock);
        shared_counter = (shared_counter + 1);
        unlock(counter_lock);
        i = (i + 1);
    }
}

function main() {
    spawn(worker());
    spawn(worker());
    wait();
    console.log(shared_counter);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
