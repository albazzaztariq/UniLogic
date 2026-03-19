const fmod = (x, y) => x % y;
const pow = Math.pow;
const sqrt = Math.sqrt;

function main() {
    let s = sqrt(625.0);
    console.log(s);
    let p = pow(2.0, 10.0);
    console.log(p);
    let m = fmod(17.0, 5.0);
    console.log(m);
    let chained = sqrt(pow(3.0, 4.0));
    console.log(chained);
    return 0;
}


main();
