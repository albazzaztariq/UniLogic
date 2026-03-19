
function outer() {
    function add_ten(x) {
        return (x + 10);
    }
    
    function twice(x) {
        return (x * 2);
    }
    
    let a = add_ten(5);
    let b = twice(a);
    console.log(a);
    console.log(b);
    return b;
}

function main() {
    function greet() {
        return "hello from nested";
    }
    
    let result = outer();
    console.log(result);
    let msg = greet();
    console.log(msg);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
