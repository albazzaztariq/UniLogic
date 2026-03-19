function check(val) {
    if ((val < 0)) {
        console.log((-1));
        process.exit(1);
    }
    return (val * 2);
}

function main() {
    let result = check(5);
    console.log(result);
    process.exit(0);
    return 0;
}


main();
