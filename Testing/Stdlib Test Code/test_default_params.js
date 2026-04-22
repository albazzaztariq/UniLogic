
function greet(name, greeting = "Hello") {
    return "{greeting}, {name}!";
}

function add(a, b = 10) {
    return (a + b);
}

function main() {
    console.log(greet("World", "Hello"));
    console.log(greet("World", "Hi"));
    console.log(add(5, 10));
    console.log(add(5, 20));
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
