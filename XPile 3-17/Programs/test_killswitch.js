// @dr safety = checked

function __ul_checked_access(arr, idx, name, line) {
    if (idx < 0 || idx >= arr.length) {
        process.stderr.write(`bounds check error: ${name}[${idx}] out of range (size ${arr.length}) at line ${line}\n`);
        process.exit(1);
    }
    return arr[idx];
}


function main() {
    let index = 3;
    let length = 10;
    if (!((index < length))) {
        process.stderr.write("killswitch failed at line 9\n");
        process.exit(1);
    }
    console.log("test1: passed");
    if (!((length > 0))) {
        process.stderr.write("killswitch failed at line 13\n");
        process.exit(1);
    }
    console.log("test2: passed");
    let x = 5;
    if (!((x === 5))) {
        process.stderr.write("killswitch failed at line 19\n");
        process.exit(1);
    }
    console.log("test3: passed");
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
