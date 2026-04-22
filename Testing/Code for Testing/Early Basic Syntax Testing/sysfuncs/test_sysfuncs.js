
function main() {
    let t = Math.floor(Date.now() / 1000);
    if ((t > 0)) {
        console.log("time ok");
    }
    let c = (typeof process !== 'undefined' ? Number(process.hrtime.bigint()) : Math.floor(performance.now() * 1e6));
    if ((c > 0)) {
        console.log("clock ok");
    }
    undefined /* random_seed not supported in JS */;
    let r = Math.random();
    if ((r >= 0.0)) {
        if ((r <= 1.0)) {
            console.log("random ok");
        }
    }
    let ri = (Math.floor(Math.random() * (10 - 1 + 1)) + 1);
    if ((ri >= 1)) {
        if ((ri <= 10)) {
            console.log("random_int ok");
        }
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
