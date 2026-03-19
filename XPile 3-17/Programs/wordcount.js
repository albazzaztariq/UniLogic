function str_char_at(s, i) { return i >= 0 && i < s.length ? s[i] : ''; }
function strlen(s) { return s.length; }


function wc(text) {
    let len = strlen(text);
    let lines = 0;
    let words = 0;
    let chars = len;
    let in_word = false;
    for (const i of __ul_range(len)) {
        let ch = str_char_at(text, i);
        let is_space = false;
        if ((ch === " ")) {
            is_space = true;
        }
        if ((ch === "\t")) {
            is_space = true;
        }
        if ((ch === "r")) {
            is_space = true;
        }
        if ((ch === "\n")) {
            is_space = true;
            lines = (lines + 1);
        }
        if ((is_space === false)) {
            if ((in_word === false)) {
                words = (words + 1);
            }
            in_word = true;
        } else {
            in_word = false;
        }
    }
    console.log("  {lines} {words} {chars}");
}

function main() {
    console.log("Test 1: empty string");
    wc("");
    console.log("Test 2: single word");
    wc("hello");
    console.log("Test 3: simple sentence");
    wc("hello world");
    console.log("Test 4: two lines");
    wc("The quick brown fox\njumps over the lazy dog\n");
    console.log("Test 5: paragraphs with blank line");
    wc("First line\nSecond line\n\nFourth line after blank\n");
    console.log("Test 6: whitespace only");
    wc("   \n\n   \n");
    return 0;
}


function __ul_range(...args) {
    let start, end, step;
    if (args.length === 1) { start = 0; end = args[0]; step = 1; }
    else if (args.length === 2) { start = args[0]; end = args[1]; step = 1; }
    else { start = args[0]; end = args[1]; step = args[2]; }
    const result = [];
    for (let i = start; step > 0 ? i < end : i > end; i += step) result.push(i);
    return result;
}
function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
