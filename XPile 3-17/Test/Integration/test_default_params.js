function greet(name, greeting = "Hello") {
    return `${greeting}, ${name}!`;
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


main();
