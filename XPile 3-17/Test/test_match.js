function test_int_match() {
    let x = 2;
    switch (x) {
        case 1:
            console.log("one");
            break;
        case 2:
            console.log("two");
            break;
        case 3:
            console.log("three");
            break;
        default:
            console.log("other");
            break;
    }
}

function test_default() {
    let y = 99;
    switch (y) {
        case 1:
            console.log("one");
            break;
        case 2:
            console.log("two");
            break;
        default:
            console.log("default hit");
            break;
    }
}

function test_match_expr() {
    let val = (3 + 2);
    switch (val) {
        case 4:
            console.log("four");
            break;
        case 5:
            console.log("five");
            break;
        case 6:
            console.log("six");
            break;
    }
}

function test_match_negative() {
    let n = (-1);
    switch (n) {
        case (-1):
            console.log("negative one");
            break;
        case 0:
            console.log("zero");
            break;
        case 1:
            console.log("positive one");
            break;
    }
}

function main() {
    test_int_match();
    test_default();
    test_match_expr();
    test_match_negative();
    return 0;
}


main();
