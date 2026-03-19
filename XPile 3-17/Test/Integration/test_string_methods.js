function strlen(s) { return s.length; }
function strstr(s, sub) { const i = s.indexOf(sub); return i >= 0 ? s.slice(i) : null; }

function str_len(s) {
    return strlen(s);
}

function str_contains(s, sub) {
    let found = strstr(s, sub);
    return (found !== null);
}

function main() {
    let s = "Hello World";
    let len = str_len(s);
    console.log(len);
    let has = str_contains(s, "World");
    console.log(has);
    return 0;
}


main();
