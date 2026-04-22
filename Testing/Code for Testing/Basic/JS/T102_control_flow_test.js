
function main() {
    let x = 15;
    if ((x > 10)) {
        console.log("greater");
    } else {
        console.log("not greater");
    }
    let val = 50;
    if ((val > 100)) {
        console.log("big");
    } else {
        if ((val > 25)) {
            console.log("medium");
        } else {
            console.log("small");
        }
    }
    let code = 2;
    switch (code) {
        case 1:
            console.log("one");
            break;
        case 2:
            console.log("two");
            break;
        case 3:
            console.log("three");
            break;
    }
    let i = 0;
    let total = 0;
    while ((i < 5)) {
        total = (total + i);
        i = (i + 1);
    }
    console.log(total);
    let sum = 0;
    for (let j = 1; (j <= 5); j++) {
        sum = (sum + j);
    }
    console.log(sum);
    let nums = [10, 20, 30];
    let arr_sum = 0;
    for (const n of nums) {
        arr_sum = (arr_sum + n);
    }
    console.log(arr_sum);
    let count = 0;
    do {
        count = (count + 1);
    } while ((count < 3));
    console.log(count);
    let last = 0;
    let k = 0;
    while ((k < 100)) {
        if ((k === 5)) {
            last = k;
            break;
        }
        k = (k + 1);
    }
    console.log(last);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
