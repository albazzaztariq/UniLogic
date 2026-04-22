function createVec2() {
    return { x: 0, y: 0 };
}


function vec_add(a, b) {
    let result = createVec2();
    result.x = (a.x + b.x);
    result.y = (a.y + b.y);
    return result;
}

function main() {
    let v1 = createVec2();
    v1.x = 1;
    v1.y = 2;
    let v2 = createVec2();
    v2.x = 3;
    v2.y = 4;
    let v3 = vec_add(v1, v2);
    console.log(v3.x);
    console.log(v3.y);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
