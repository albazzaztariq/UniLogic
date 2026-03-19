const sqrt = Math.sqrt;

function main() {
    let x = 5;
    x = (x + 10);
    x = (x * 2);
    x = (x - 1);
    console.log(x);
    let c = 0;
    c = (c + 1);
    c = (c + 1);
    c = (c + 1);
    c = (c - 1);
    console.log(c);
    let _t0 = sqrt(256.0);
    let _t1 = sqrt(_t0);
    let v = sqrt(_t1);
    console.log(v);
    let raw = 97;
    let _t2 = Number(raw);
    let d = Number(_t2);
    console.log(d);
    let a = 10;
    let b = 20;
    let c2 = 30;
    let d2 = 40;
    let _t3 = ((a > 0) && (b > 0));
    let _t4 = (_t3 && (c2 > 0));
    if ((_t4 && (d2 > 0))) {
        console.log(1);
    }
    let _t5 = ((a > 0) && (b > 100));
    if ((_t5 && (c2 > 0))) {
        console.log(999);
    }
    console.log(0);
    return 0;
}


main();
