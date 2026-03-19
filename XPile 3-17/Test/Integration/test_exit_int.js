function validate(x) {
    if ((x < 0)) {
        return (-1);
    }
    return x;
}

function main() {
    let val = validate(10);
    if ((val < 0)) {
        console.log(0);
        return 1;
    }
    console.log(val);
    return 0;
}


main();
