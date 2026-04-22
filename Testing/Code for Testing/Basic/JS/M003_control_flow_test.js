
function classify(score) {
    if ((score > 90)) {
        return "A";
    } else {
        if ((score > 80)) {
            return "B";
        } else {
            if ((score > 70)) {
                return "C";
            } else {
                return "F";
            }
        }
    }
}

function main() {
    console.log(classify(95));
    console.log(classify(85));
    console.log(classify(72));
    console.log(classify(50));
    let count = 0;
    while ((count < 5)) {
        count++;
    }
    console.log(count);
    let nums = [10, 20, 30, 40, 50];
    let total = 0;
    for (const n of nums) {
        total += n;
    }
    console.log(total);
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
        default:
            console.log("other");
            break;
    }
    let i = 0;
    while ((i < 100)) {
        if ((i === 3)) {
            break;
        }
        i++;
    }
    console.log(i);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
