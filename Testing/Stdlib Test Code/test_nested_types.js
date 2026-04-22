function createInner() {
    return { value: 0, code: 0 };
}

function createMiddle() {
    return { label: "", core: createInner() };
}

function createOuter() {
    return { id: 0, mid: createMiddle() };
}


function set_inner(v, c) {
    let i = createInner();
    i.value = v;
    i.code = c;
    return i;
}

function main() {
    let o = createOuter();
    o.id = 1;
    o.mid.label = "test";
    o.mid.core.value = 42;
    o.mid.core.code = 99;
    console.log(o.id);
    console.log(o.mid.label);
    console.log(o.mid.core.value);
    console.log(o.mid.core.code);
    let o2 = createOuter();
    o2.id = 2;
    o2.mid.label = "second";
    o2.mid.core.value = 100;
    o2.mid.core.code = 200;
    console.log(o2.id);
    console.log(o2.mid.label);
    console.log(o2.mid.core.value);
    console.log(o2.mid.core.code);
    o.mid.core.value = 777;
    console.log(o.mid.core.value);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
