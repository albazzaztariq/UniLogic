
function main() {
    let t = Math.floor(Date.now() / 1000);
    if ((t > 0)) {
        console.log(1);
    }
    let c = (typeof process !== 'undefined' ? Number(process.hrtime.bigint()) : Math.floor(performance.now() * 1e6));
    if ((c > 0)) {
        console.log(1);
    }
    undefined /* random_seed not supported in JS */;
    let r1 = (Math.floor(Math.random() * (100 - 1 + 1)) + 1);
    if (((r1 >= 1) && (r1 <= 100))) {
        console.log(1);
    }
    undefined /* random_seed not supported in JS */;
    let f = Math.random();
    if (((f >= 0.0) && (f < 1.0))) {
        console.log(1);
    }
    let path = (typeof process !== 'undefined' ? (process.env["PATH"] || '') : '');
    if ((path.length > 0)) {
        console.log(1);
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
