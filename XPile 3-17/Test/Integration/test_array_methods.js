function array_sum(arr, n) {
    let total = 0;
    let i = 0;
    while ((i < n)) {
        total = (total + arr[i]);
        i = (i + 1);
    }
    return total;
}

function array_contains(arr, n, val) {
    let i = 0;
    while ((i < n)) {
        if ((arr[i] === val)) {
            return true;
        }
        i = (i + 1);
    }
    return false;
}

function main() {
    let nums = [3, 1, 4, 1, 5];
    let total = array_sum(nums, nums.length);
    console.log(total);
    let found = array_contains(nums, nums.length, 4);
    console.log(found);
    let nope = array_contains(nums, nums.length, 9);
    console.log(nope);
    return 0;
}


main();
