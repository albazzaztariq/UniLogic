function createPoint() {
    return { x: 0, y: 0 };
}

function createRectangle() {
    return { top_left: createPoint(), bottom_right: createPoint() };
}


function make_point(px, py) {
    let p = createPoint();
    p.x = px;
    p.y = py;
    return p;
}

function distance_sq(a, b) {
    let dx = (b.x - a.x);
    let dy = (b.y - a.y);
    return ((dx * dx) + (dy * dy));
}

function main() {
    let p1 = createPoint();
    p1.x = 3;
    p1.y = 4;
    console.log(p1.x);
    console.log(p1.y);
    let p2 = make_point(10, 20);
    console.log(p2.x);
    console.log(p2.y);
    let dist = distance_sq(p1, p2);
    console.log(dist);
    let r = createRectangle();
    r.top_left = make_point(0, 0);
    r.bottom_right = make_point(100, 50);
    console.log(r.top_left.x);
    console.log(r.bottom_right.y);
    p1.x = 99;
    console.log(p1.x);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
