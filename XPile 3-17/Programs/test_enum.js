const Priority = Object.freeze({low: 1, medium: 2, high: 3});
const Color = Object.freeze({red: 10, green: 20, blue: 30});

function main() {
    let p = Priority.high;
    console.log(p);
    let total = ((Priority.low + Priority.medium) + Priority.high);
    console.log(total);
    if ((p === Priority.high)) {
        console.log(1);
    }
    let c = Color.green;
    console.log(c);
    let val = Priority.medium;
    switch (val) {
        case 1:
            console.log(100);
            break;
        case 2:
            console.log(200);
            break;
        case 3:
            console.log(300);
            break;
    }
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
