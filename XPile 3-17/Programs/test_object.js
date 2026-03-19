class Counter {
    constructor(count = 0) {
        this.count = count;
    }
    get_count() {
        return this.count;
    }
    increment() {
        this.count = (this.count + 1);
        return this.count;
    }
}

class DoubleCounter extends Counter {
    constructor(count = 0) {
        super(count);
    }
    double_increment() {
        this.count = (this.count + 2);
        return this.count;
    }
}


function main() {
    let c = new Counter();
    c.count = 0;
    console.log(c.get_count());
    console.log(c.increment());
    console.log(c.increment());
    console.log(c.get_count());
    let dc = new DoubleCounter();
    dc.count = 10;
    console.log(dc.get_count());
    console.log(dc.double_increment());
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
