function createPoint() {
    return { x: 0, y: 0 };
}

function createPerson() {
    return { name: "", age: 0, location: createPoint() };
}


function print_point(p) {
    console.log(p.x);
    console.log(p.y);
}

function make_point(x, y) {
    let result = createPoint();
    result.x = x;
    result.y = y;
    return result;
}

function main() {
    let p = createPoint();
    p.x = 10;
    p.y = 20;
    console.log(p.x);
    console.log(p.y);
    let bob = createPerson();
    bob.name = "Bob";
    bob.age = 30;
    bob.location.x = 100;
    bob.location.y = 200;
    console.log(bob.name);
    console.log(bob.age);
    console.log(bob.location.x);
    console.log(bob.location.y);
    print_point(p);
    let q = createPoint();
    q = make_point(5, 15);
    console.log(q.x);
    console.log(q.y);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
