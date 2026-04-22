// @dr safety = checked

function __ul_checked_access(arr, idx, name, line) {
    if (idx < 0 || idx >= arr.length) {
        process.stderr.write(`bounds check error: ${name}[${idx}] out of range (size ${arr.length}) at line ${line}\n`);
        process.exit(1);
    }
    return arr[idx];
}

function main() {
    let arr = [1, 2, 3];
    console.log(__ul_checked_access(arr, 1, "arr", 7));
    console.log(__ul_checked_access(arr, 10, "arr", 8));
    return 0;
}


main();
