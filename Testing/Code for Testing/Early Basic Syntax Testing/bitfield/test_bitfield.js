// bit field: ready = 1 bits
// bit field: fault = 1 bits
// bit field: mode = 3 bits
// bit field: priority = 3 bits
function createStatusFlags() {
    return { ready: 0, fault: 0, mode: 0, priority: 0 };
}


function main() {
    let flags = createStatusFlags();
    flags.ready = 1;
    flags.fault = 0;
    flags.mode = 5;
    flags.priority = 3;
    console.log(flags.ready);
    console.log(flags.fault);
    console.log(flags.mode);
    console.log(flags.priority);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
