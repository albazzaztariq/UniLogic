// @dr memory = gc

class Node {
    constructor(value = 0, label = "") {
        this.value = value;
        this.label = label;
    }
}


function make_nodes(count) {
    let i = 0;
    let total = 0;
    while ((i < count)) {
        let n = new Node();
        n.value = i;
        n.label = "node";
        total = (total + n.value);
        i = (i + 1);
    }
    return total;
}

function main() {
    let result = make_nodes(100);
    console.log(result);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
