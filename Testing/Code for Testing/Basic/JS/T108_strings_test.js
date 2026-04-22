function str_char_at(s, i) { return i >= 0 && i < s.length ? s[i] : ''; }
function str_substr(s, start, len) { return s.substr(start, len); }
function strlen(s) { return s.length; }


function main() {
    let hello = (("Hello" + " ") + "World");
    console.log(hello);
    let len = strlen(hello);
    console.log(len);
    let first = str_char_at(hello, 0);
    console.log(first);
    let sub = str_substr(hello, 0, 5);
    console.log(sub);
    let greeting = "Good Morning";
    let part = greeting.slice(0, 4);
    console.log(part);
    let a = "abc";
    let b = "abc";
    if ((a === b)) {
        console.log("equal");
    }
    let c = "xyz";
    if ((a !== c)) {
        console.log("not equal");
    }
    let sentence = "the quick brown fox";
    if (sentence.includes("quick")) {
        console.log("found quick");
    }
    let result = "";
    let i = 0;
    while ((i < 3)) {
        result = (result + String(i));
        i = (i + 1);
    }
    console.log(result);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
