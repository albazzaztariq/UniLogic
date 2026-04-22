function array_get(arr, n, idx) {
    if ((idx < 0)) {
        return 0;
    }
    if ((idx >= n)) {
        return 0;
    }
    return arr[idx];
}

function array_set(arr, n, idx, val) {
    if ((idx < 0)) {
        return 0;
    }
    if ((idx >= n)) {
        return 0;
    }
    arr[idx] = val;
    return 1;
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

function array_index_of(arr, n, val) {
    let i = 0;
    while ((i < n)) {
        if ((arr[i] === val)) {
            return i;
        }
        i = (i + 1);
    }
    return (0 - 1);
}

function array_sum(arr, n) {
    let total = 0;
    let i = 0;
    while ((i < n)) {
        total = (total + arr[i]);
        i = (i + 1);
    }
    return total;
}

function array_min(arr, n) {
    let m = arr[0];
    let i = 1;
    while ((i < n)) {
        if ((arr[i] < m)) {
            m = arr[i];
        }
        i = (i + 1);
    }
    return m;
}

function array_max(arr, n) {
    let m = arr[0];
    let i = 1;
    while ((i < n)) {
        if ((arr[i] > m)) {
            m = arr[i];
        }
        i = (i + 1);
    }
    return m;
}

function array_reverse(arr, n) {
    let lo = 0;
    let hi = (n - 1);
    while ((lo < hi)) {
        let tmp = arr[lo];
        arr[lo] = arr[hi];
        arr[hi] = tmp;
        lo = (lo + 1);
        hi = (hi - 1);
    }
    return 0;
}

function array_sort(arr, n) {
    let i = 1;
    while ((i < n)) {
        let key = arr[i];
        let j = (i - 1);
        while ((j >= 0)) {
            if ((arr[j] > key)) {
                arr[(j + 1)] = arr[j];
                j = (j - 1);
            } else {
                break;
            }
        }
        arr[(j + 1)] = key;
        i = (i + 1);
    }
    return 0;
}

function array_count(arr, n, val) {
    let count = 0;
    let i = 0;
    while ((i < n)) {
        if ((arr[i] === val)) {
            count = (count + 1);
        }
        i = (i + 1);
    }
    return count;
}

function main() {
    let nums = [10, 30, 20, 50, 40];
    let val = array_get(nums, 5, 2);
    console.log(val);
    array_set(nums, 5, 2, 99);
    console.log(nums[2]);
    let has50 = array_contains(nums, 5, 50);
    console.log(has50);
    let has77 = array_contains(nums, 5, 77);
    console.log(has77);
    let idx = array_index_of(nums, 5, 50);
    console.log(idx);
    let data = [1, 2, 3, 4];
    let total = array_sum(data, 4);
    console.log(total);
    let lo = array_min(data, 4);
    console.log(lo);
    let hi = array_max(data, 4);
    console.log(hi);
    let reps = [1, 2, 3, 2, 2, 1];
    let twos = array_count(reps, 6, 2);
    console.log(twos);
    let rev = [1, 2, 3, 4];
    array_reverse(rev, 4);
    console.log(rev[0]);
    console.log(rev[1]);
    console.log(rev[2]);
    console.log(rev[3]);
    let unsorted = [5, 3, 1, 4, 2];
    array_sort(unsorted, 5);
    console.log(unsorted[0]);
    console.log(unsorted[1]);
    console.log(unsorted[2]);
    console.log(unsorted[3]);
    console.log(unsorted[4]);
    return 0;
}


main();
