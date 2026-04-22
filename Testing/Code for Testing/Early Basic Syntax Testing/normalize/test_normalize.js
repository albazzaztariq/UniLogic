const sqrt = Math.sqrt;


function main() {
    let counter = 10;
    counter += 5;
    counter *= 2;
    let i = 0;
    i++;
    i++;
    i--;
    console.log(counter);
    console.log(i);
    let val = 144.0;
    let deep = sqrt(sqrt(val));
    console.log(deep);
    let x = 42;
    let y = Number(Number(x));
    console.log(y);
    let a = 1;
    let b = 2;
    let c = 3;
    if ((((a > 0) && (b > 0)) && (c > 0))) {
        console.log(a);
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
