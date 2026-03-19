
const NULL_PTR = 0;
function list_push(values, nexts, head, next_free, val) {
    values[next_free] = val;
    nexts[next_free] = head;
    return next_free;
}

function list_append(values, nexts, head, next_free, val) {
    values[next_free] = val;
    nexts[next_free] = NULL_PTR;
    if ((head === NULL_PTR)) {
        return next_free;
    }
    let cur = head;
    while ((nexts[cur] !== NULL_PTR)) {
        cur = nexts[cur];
    }
    nexts[cur] = next_free;
    return head;
}

function list_len(nexts, head) {
    let count = 0;
    let cur = head;
    while ((cur !== NULL_PTR)) {
        count = (count + 1);
        cur = nexts[cur];
    }
    return count;
}

function list_contains(values, nexts, head, target) {
    let cur = head;
    while ((cur !== NULL_PTR)) {
        if ((values[cur] === target)) {
            return true;
        }
        cur = nexts[cur];
    }
    return false;
}

function list_remove(values, nexts, head, target) {
    if ((head === NULL_PTR)) {
        return NULL_PTR;
    }
    if ((values[head] === target)) {
        return nexts[head];
    }
    let prev = head;
    let cur = nexts[head];
    while ((cur !== NULL_PTR)) {
        if ((values[cur] === target)) {
            nexts[prev] = nexts[cur];
            return head;
        }
        prev = cur;
        cur = nexts[cur];
    }
    return head;
}

function list_print(values, nexts, head) {
    let cur = head;
    while ((cur !== NULL_PTR)) {
        console.log(values[cur]);
        cur = nexts[cur];
    }
}

function list_reverse(values, nexts, head) {
    let prev = NULL_PTR;
    let cur = head;
    while ((cur !== NULL_PTR)) {
        let next = nexts[cur];
        nexts[cur] = prev;
        prev = cur;
        cur = next;
    }
    return prev;
}

function list_get(values, nexts, head, n) {
    let cur = head;
    let idx = 0;
    while ((cur !== NULL_PTR)) {
        if ((idx === n)) {
            return values[cur];
        }
        idx = (idx + 1);
        cur = nexts[cur];
    }
    return (-1);
}

function list_sum(values, nexts, head) {
    let total = 0;
    let cur = head;
    while ((cur !== NULL_PTR)) {
        total = (total + values[cur]);
        cur = nexts[cur];
    }
    return total;
}

function main() {
    let values = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    let nexts = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    let nf = 1;
    let head = NULL_PTR;
    head = list_push(values, nexts, head, nf, 10);
    nf = (nf + 1);
    head = list_push(values, nexts, head, nf, 20);
    nf = (nf + 1);
    head = list_push(values, nexts, head, nf, 30);
    nf = (nf + 1);
    console.log("After push 10, 20, 30:");
    list_print(values, nexts, head);
    head = list_append(values, nexts, head, nf, 40);
    nf = (nf + 1);
    head = list_append(values, nexts, head, nf, 50);
    nf = (nf + 1);
    console.log("After append 40, 50:");
    list_print(values, nexts, head);
    let len = list_len(nexts, head);
    console.log("Length:");
    console.log(len);
    let has20 = list_contains(values, nexts, head, 20);
    let has99 = list_contains(values, nexts, head, 99);
    console.log("Contains 20:");
    console.log(has20);
    console.log("Contains 99:");
    console.log(has99);
    head = list_remove(values, nexts, head, 20);
    console.log("After remove 20:");
    list_print(values, nexts, head);
    head = list_reverse(values, nexts, head);
    console.log("After reverse:");
    list_print(values, nexts, head);
    let sum = list_sum(values, nexts, head);
    console.log("Sum:");
    console.log(sum);
    let second = list_get(values, nexts, head, 1);
    console.log("Element at index 1:");
    console.log(second);
    let final_len = list_len(nexts, head);
    console.log("Final length:");
    console.log(final_len);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
