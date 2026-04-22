
function main() {
    let sensor_value = 0;  // nocache (volatile)
    sensor_value = 42;
    console.log(sensor_value);
    let counter = 10;  // nocache (volatile)
    counter += 5;
    console.log(counter);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
